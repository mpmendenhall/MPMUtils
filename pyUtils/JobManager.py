#! /bin/env python3
## @file JobManager.py job submission manager for cluster computing environments

import sqlite3
import subprocess
import os
from optparse import OptionParser, SUPPRESS_HELP
import time
import multiprocessing
import datetime

jobstates = {"Idle":2,"Starting":3,"Running":3,"Completed":4,"Unknown":5,"Removed":6,"Bundled":7}
statenames = {-1:"hold", 0:"waiting", 1:"sumitted", 2:"queued", 3:"running", 4: "done", 6:"removed", 7:"bundled" }
def statename(i): return statenames.get(i,"status %i"%i)
dbfile = None # path to database file
jobscript_dir = os.environ["HOME"]+"/jobs/"
resource_ids = { } # cached resource IDs by name

def connect_JobsDB(fname):
    """Get cursor and connection for Jobs DB"""
    conn = sqlite3.connect(fname)
    curs = conn.cursor()
    curs.execute("PRAGMA foreign_keys = ON")
    return curs,conn

##
## DB commands
##

def new_job(curs, jname):
    """Return job_id for a new empty job"""
    curs.execute("INSERT INTO jobs(name) VALUES (?)", (jname,))
    return curs.lastrowid

def set_job_status(curs, jid, status = None, ret = None, walltime = None):
    """Set job status information"""
    info = []
    qs = []
    if status is not None: info.append(status); qs.append("status = ?")
    if ret is not None: info.append(ret); qs.append("return_code = ?")
    if walltime is not None:
        if walltime == "auto": info.append(time.time()); qs.append("use_walltime = ? - t_submit")
        else: info.append(walltime); qs.append("use_walltime = ?")
    if not info: return
    info.append(jid)
    curs.execute("UPDATE jobs SET "+", ".join(qs)+" WHERE job_id = ?", info)

def summarize_DB_runstatus(curs):
    """Print summary counts by run status in DB"""
    print("Job submission database summary:")
    for i in range(11):
        curs.execute("SELECT COUNT(*) FROM jobs WHERE status=?",(i,))
        n = curs.fetchone()[0]
        if n: print("\t%s: %i jobs"%(statename(i),n))

def clear_completed(conn, clearlogs = True):
    curs = conn.cursor()
    if clearlogs:
        curs.execute("SELECT jobfile, outlog FROM jobs WHERE status=4 OR status=6 OR status=7")
        for r in curs.fetchall():
            for f in r:
                if(f): os.system("rm -f "+f)
    curs.execute("DELETE FROM jobs WHERE status=4 OR status=6 OR status=7")
    conn.commit()

##
## resource management
##

def find_resource_id(curs, name):
    """Find named resource, if it exists"""
    if name in resource_ids: return resource_ids[name]
    curs.execute("SELECT resource_id FROM resources WHERE name = ?", (name,))
    res = curs.fetchall()
    resource_ids[name] = res[0][0] if len(res) == 1 else None
    return resource_ids[name]

def create_resource(curs, name, descrip, lim = 1.0):
    """Create named resource; return id"""
    curs.execute("INSERT INTO resources(name,descrip,available) VALUES (?,?,?)", (name, descrip, lim))
    resource_ids[name] = curs.lastrowid
    return resource_ids[name]

def get_resource_id(curs, name, descrip, lim = 1.0):
    """Find or create new named resource; return ID"""
    eid = find_resource_id(curs, name)
    if eid is not None: return eid
    return create_resource(curs, name, descrip, lim)

def get_walltime_requested(curs, jid):
    """Get job computation time request"""
    walltime_resource_id = get_resource_id(curs, "walltime", "run wall time [s]", 1e9)
    curs.execute("SELECT quantity FROM resource_use WHERE job_id = ? AND resource_id = ?", (jid, walltime_resource_id))
    rs = curs.fetchall()
    if len(rs) != 1: return None
    return rs[0][0]

def get_cores_requested(curs, jid):
    """Get job cores requirement"""
    cores_resource_id = get_resource_id(curs, "cores", "number of cores")
    curs.execute("SELECT quantity FROM resource_use WHERE job_id = ? AND resource_id = ?", (jid, cores_resource_id))
    rs = curs.fetchall()
    if len(rs) != 1: return None
    return rs[0][0]

def set_resource_limit(curs, rid, lim):
    """Set resources table limit"""
    curs.execute("UPDATE resources SET available = ? WHERE resource_id = ?", (lim,rid))

def check_resource_use(curs, rid):
    """Get resource limit and use by queued/running jobs"""
    curs.execute("SELECT available FROM resources WHERE resource_id = ?", (rid,))
    avail = curs.fetchone()[0]
    curs.execute("SELECT TOTAL(quantity) FROM resource_use NATURAL JOIN jobs WHERE resource_id = ? AND 1 <= status AND status <= 3", (rid,))
    used = curs.fetchone()[0]
    return [avail,used]

def display_resource_use(curs):
    """Print current resource use"""
    print("Resources in use:")
    curs.execute("SELECT resource_id,name,descrip FROM resources")
    for rs in curs.fetchall():
        resuse = check_resource_use(curs,rs[0])
        print("\t%s [%s]: %g / %g used"%(rs[1], rs[2], resuse[1], resuse[0]))
    print("-----------------")

def set_job_resource(curs, jid, rid, qty):
    """Set resource use for a job"""
    if type(rid)==type(""): rid = get_resource_id(curs,rid,rid)
    curs.execute("INSERT INTO resource_use(job_id, resource_id, quantity) VALUES (?,?,?)", (jid, rid, qty))

def get_job_resources(curs, jid):
    """Get list of resources requested by a job"""
    curs.execute("SELECT resource_id,quantity FROM resource_use WHERE job_id = ?", (jid,))
    return curs.fetchall()

def get_possible_submissions(curs, nmax = 1000000):
    """Select list of waiting (jobs, resources) that could be submitted within resource limits"""
    jout = []
    resuse = {}
    curs.execute("SELECT job_id FROM jobs WHERE status = 0 LIMIT ?",(nmax,))
    for jid in [r[0] for r in curs.fetchall()]:
        res_ok = True
        job_rs = get_job_resources(curs, jid)
        for rs in job_rs:
            if rs[0] not in resuse: resuse[rs[0]] = check_resource_use(curs,rs[0])
            if resuse[rs[0]][0] < resuse[rs[0]][1] + rs[1]:
                res_ok = False
                break
        if not res_ok: continue
        for rs in job_rs: resuse[rs[0]][1] += rs[1]
        jout.append((jid, job_rs))
    return jout

##
## batch queue interfaces
##

def get_showq_runstatus():
    """Get queue status according to showq"""
    cmd = 'showq -u $USER'
    qdat = subprocess.getoutput(cmd)
    print(cmd)
    print(qdat)
    jstatus = []
    for l in qdat.split("\n"):
        l = l.split()
        if len(l) != 9: continue
        status = jobstates.get(l[2],5)
        jstatus.append((int(l[0]), status, int(l[3])))
    return jstatus

def checkjob(jid):
    """Get info from checkjob"""
    print("Checking job status for",jid)
    jdat = {}
    for j in subprocess.getoutput("checkjob %i"%jid).split("\n"):
        print(j)
        i = j.find(":")
        if i<1: continue
        var = j[:i].strip()
        val = j[i+1:].strip()
        if not var or not val: continue
        if var=="Completion Code": jdat["ret_code"] = int(val.split()[0])
        elif var=="WallTime":
            val = val.split(":")
            try: jdat["walltime"] = 3600*int(val[0]) + 60*int(val[1]) + int(val[2].split()[0])
            except: jdat["walltime"] = -1
        else: jdat[var]=val
    jdat["status"] = jobstates.get(jdat.get("State","unknown"),5)
    print(jdat)
    return jdat

def msub_job(curs, jid, account, qname, mcmds=["-j oe", "-V"]):
    """Submit job via msub; update DB"""

    curs.execute("SELECT name,jobfile,outlog FROM jobs WHERE job_id = ?",(jid,))
    j = curs.fetchone()
    if not j: return
    wtreq = get_walltime_requested(curs,jid)
    if wtreq is None: wtreq = 1800
    ncores = get_cores_requested(curs,jid)
    if ncores is None: ncores = 1

    # modify job script with msub commands
    jobin = open(j[1],"r").read()
    jobout = open(j[1],"w")

    jobout.write("#!/usr/bin/bash\n")
    jobout.write("#MSUB -A %s\n"%account)
    if j[2]: jobout.write("#MSUB -o %s\n"%j[2])
    if j[0]: jobout.write("#MSUB -N %s\n"%j[0])
    jobout.write("#MSUB -q %s\n"%qname)
    jobout.write("#MSUB -l nodes=%i\n"%ncores)
    jobout.write("#MSUB -l walltime=%i\n"%wtreq)
    for m in mcmds: jobout.write("#MSUB "+m+"\n")

    jobout.write(jobin)
    jobout.close()

    o = subprocess.getoutput('msub "%s"'%j[1]).strip()
    print("Job '%s' submitted as '%s'"%(j[0],o))
    qid = int(o.split()[-1].split(".")[0])
    curs.execute("UPDATE jobs SET queue_id=?, status=1, t_submit = ? WHERE job_id = ?",(qid,time.time(),jid))

def update_DB_qrunstatus(curs,jstatus):
    """Update run status in database from job status list"""
    curs.executemany("UPDATE jobs SET status=? WHERE queue_id=?",[(j[1],j[0]) for j in jstatus])

    jknown = frozenset([j[0] for j in jstatus if 1 <= j[1] <= 3])
    curs.execute("SELECT queue_id FROM jobs WHERE 1 <= status AND (status <= 3 OR status == 5)")
    missingjobs = [(j[0],checkjob(j[0])) for j in curs.fetchall() if j[0] not in jknown]
    missingjobs = [(j[1]["status"],j[1].get("ret_code",None),j[1].get("walltime",None),j[0]) for j in missingjobs]
    if missingjobs: curs.executemany("UPDATE jobs SET status=?,return_code=?,use_walltime=? WHERE queue_id=?",missingjobs)

def check_running_qjobs(conn):
    """Check (formerly) running jobs; update DB status appropriately"""
    jstatus = get_showq_runstatus()
    curs = conn.cursor()
    #conn.isolation_level = 'EXCLUSIVE'
    #conn.execute('BEGIN EXCLUSIVE')
    update_DB_qrunstatus(curs,jstatus)
    conn.commit()

def update_qstatus(conn):
    """Update database job status"""
    jstatus = get_showq_runstatus()
    curs = conn.cursor()
    #conn.isolation_level = 'EXCLUSIVE'
    #conn.execute('BEGIN EXCLUSIVE')
    update_DB_qrunstatus(curs,jstatus)
    conn.commit()

def update_and_launch_q(conn, account, qname, trickle = 0):
    """Update status; launch new jobs as available"""
    jstatus = get_showq_runstatus()

    curs = conn.cursor()

    #conn.isolation_level = 'EXCLUSIVE'
    #conn.execute('BEGIN EXCLUSIVE')

    update_DB_qrunstatus(curs,jstatus)
    conn.commit()
    jnext = get_possible_submissions(curs)
    if len(jnext):
        print("Submitting %i new jobs."%len(jnext))
        t0 = datetime.now() if trickle else None
        for j in jnext:
            mcmds=["-j oe", "-V"]
            if t0:
                t0 += datetime.timedelta(seconds=trickle)
                mcmds.append("-a "+t0.strftime("%Y%m%d%H%M.%S"))
            msub_job(curs, j[0], account, qname, mcmds)
            conn.commit()

    summarize_DB_runstatus(curs)

def cancel_queued_jobs(conn):
    """Force cancel all queued up jobs"""
    update_qstatus(conn)

    curs = conn.cursor()
    curs.execute("SELECT queue_id FROM jobs WHERE queue_id > 0 AND status <= 2")
    for j in curs.fetchall():
        os.system("mjobctl -F %i"%j)
    curs.execute("UPDATE jobs SET status = 6 WHERE status = 0")
    conn.commit()
    summarize_DB_runstatus(curs)


##
## local jobs
##

def check_running_localjobs(curs):
    """Check (formerly) running jobs; update DB status appropriately"""
    curs.execute("SELECT job_id,queue_id FROM jobs WHERE status = 3")
    for j in curs.fetchall():
        try: os.kill(j[1], 0)
        except: curs.execute("UPDATE jobs SET status = 4, return_code = -999 WHERE job_id = ? AND status = 3", (j[0],))

##
## job launching
##

def upload_onejob(curs, name, infile, logfile, res_list):
    """Upload one job to run; return DB job identifier"""
    curs.execute("INSERT INTO jobs(name,jobfile,outlog,status) VALUES (?,?,?,0)", (name, infile, logfile))
    jid = curs.lastrowid
    for rs in res_list: set_job_resource(curs, jid, rs[0], rs[1])
    return jid

def upload_jobs(curs,joblist):
    """Upload a list of jobs to run; return list of database IDs"""
    # job specifier: (name,input,log,res_list)
    res_ids = {}
    jids = []
    print("Uploading jobs")
    for j in joblist:
        print("\t",j)
        jid = upload_onejob(curs, j[0], j[1], j[2], [])
        jids.append(jid)
        for rs in j[-1]:
            if rs[0] not in res_ids: res_ids[rs[0]] = get_resource_id(curs,rs[0],rs[0])
            set_job_resource(curs, jid, res_ids[rs[0]], rs[1])
    return jids

def make_upload_jobs(curs, jname, jcmds, res_use):
    """Generate jobfiles for "one liners" and upload to DB"""
    # jcmds = (contents, logfile)
    jobdir = jobscript_dir + "/" + jname
    os.makedirs(jobdir, exist_ok=True)
    joblist = []
    for (n,jc) in enumerate(jcmds):
        jfl = jobdir + "/job_%i.sh"%n
        logfl = jobdir+"/log_%i.txt"%n
        if type(jc)==type(""): open(jfl,"w").write(jc)
        else:
            logfl = jc[1]
            open(jfl,"w").write(jc[0])
        joblist.append((jname, jfl, logfl, res_use))
    return upload_jobs(curs,joblist)

def make_job_script(curs, jname, jcmd, res_use):
    """Create script and logfile for "one-liner" job"""
    jid = new_job(curs, jname)
    jobdir = jobscript_dir + "/" + jname
    os.makedirs(jobdir, exist_ok=True)
    jfl = jobdir + "/job_%i.sh"%jid
    open(jfl,"w").write(jcmd+"\n")
    logfl = jobdir+"/log_%i.txt"%jid
    for rs in res_use: set_job_resource(curs, jid, rs[0], rs[1])
    curs.execute("UPDATE jobs SET jobfile = ?, outlog = ?, status = 0 WHERE job_id = ?", (jfl, logfl, jid))
    return jid

def make_test_jobs(curs, njobs):
    """Generate test run batch"""
    for i in range(njobs):
        make_job_script(curs, "test", 'echo "Hello world %i!"\nsleep 25\necho "Goodbye!"'%i, [("walltime",30), ("cores",1)])

def choose_bundles(ts, tmax, nmax):
    """Determine how to split list of estimated run times into bundles... dumb algorithm"""
    bout = []
    bnew = []
    tsum = 0
    for i in range(len(ts)):
        if len(bnew)==nmax or tsum + ts[i] > tmax:
            if len(bnew) >= 2: bout.append(bnew)
            bnew = [i,]
            tsum = ts[i]
        else:
            tsum += ts[i]
            bnew.append(i)
    if len(bnew) >= 2: bout.append(bnew)
    return bout

def make_bundle_jobs(curs,nnodes,jname,bundledir,jlist,tmax,nmax):
    """Build and upload bundling jobs; called from 'rebundle'"""
    print("Bundling %i %i-node jobs\n"%(len(jlist),nnodes))
    for bn,jb in enumerate(choose_bundles([j[4] for j in jlist], tmax, nmax)):
        bfname = bundledir+"/bundle_"+jname+"_%i.sh"%bn
        bjobfile = open(bfname, "w")
        tsum = 0
        for i in jb:
            tsum += jlist[i][4]
            bcmd = "source %s"%jlist[i][1]
            if jlist[i][2]: bcmd += " > %s 2>&1"%jlist[i][2]
            bjobfile.write(bcmd+"\n")
        bjobfile.close()
        bundle_id = upload_onejob(curs, "bundle_"+jname, bfname, bfname+"_log.txt", nnodes, tsum)
        for i in jb: curs.execute("UPDATE jobs SET status=7,associated=? WHERE job_id=?",(bundle_id,jlist[i][0]))

def rebundle(curs,jname,bundledir,tmax,nmax=1000):
    """Bundle small jobs together into longer-time units"""
    assert False # TODO est_walltime
    curs.execute("SELECT job_id,jobfile,outlog,n_nodes,est_walltime FROM jobs WHERE status=0 AND name=? AND est_walltime < ?",(jname,tmax))
    njobs = {}
    for r in curs.fetchall(): njobs.setdefault(r[3],[]).append(r)
    for n in njobs: make_bundle_jobs(curs, n, jname, bundledir, njobs[n], tmax, nmax)

def run_local(curs, trickle = 0):
    """Run jobs on local node; return number of jobs submitted."""
    check_running_localjobs(curs)
    js = get_possible_submissions(curs, 2*multiprocessing.cpu_count())
    if not len(js): return 0
    print("Launching", len(js), "jobs locally.")

    for j in js:
        curs.execute("SELECT jobfile,outlog FROM jobs WHERE job_id = ?", (j[0],))
        r = curs.fetchone()
        os.system("chmod +x "+r[0])
        jcmd = r[0]
        if r[1]: jcmd += " > %s 2>&1"%r[1]
        jcmd += "; python3 %s --db %s --jid %i --setreturn $? --setstatus 4"%(__file__, dbfile, j[0])
        print(jcmd)
        curs.execute("UPDATE jobs SET status = 1, t_submit = ? WHERE job_id = ?", (time.time(), j[0]))
        pid = subprocess.Popen([jcmd,], shell=True, preexec_fn=(lambda : os.nice(15))).pid
        curs.execute("UPDATE jobs SET status = 3, queue_id = ? WHERE job_id = ? AND status != 4", (pid, j[0]))
        if trickle: time.sleep(trickle)
    return len(js)

def cycle_launcher(conn, trickle=0, runlocal=False, twait=15, account=None, qname=None):
    curs = conn.cursor()
    while 1:
        curs.execute("SELECT COUNT(*) FROM jobs WHERE status=0")
        nleft = curs.fetchone()[0]
        print(nleft, "jobs left to be run.")
        if not nleft: break
        if runlocal: run_local(curs, trickle)
        else: update_and_launch_q(conn, account, qname, trickle)
        conn.commit()
        time.sleep(twait)
    conn.commit()

###################
###################
###################

def run_commandline():
    parser = OptionParser()

    parser.add_option("--account",  help="submission billing account")
    parser.add_option("--queue",    help="submission queue")
    parser.add_option("--limit",    type="int", default=multiprocessing.cpu_count(), help="concurrent jobs limit")
    parser.add_option("--trickle",  type="float", help="time delay [s] between nominal run starts")
    parser.add_option("--db",       help="jobs database")

    parser.add_option("--launch",   action="store_true", help="update and launch")
    parser.add_option("--cycle",    type="float", help="continuously re-check/launch jobs at specified interval")
    parser.add_option("--status",   action="store_true", help="update and display status")
    parser.add_option("--cancel",   action="store_true", help="cancel queued jobs")
    parser.add_option("--clear",    action="store_true", help="clear completed jobs")
    parser.add_option("--jobfile",  help="run one-liners in file")
    parser.add_option("--walltime", type="int", help="wall time for 1-liner jobs in seconds")
    parser.add_option("--nodes",    type="int", default=1, help="nodes for 1-liner jobs")
    parser.add_option("--bundle",   help="bundle job name; specify bundled walltime")
    parser.add_option("--runlocal", action="store_true", help="run all waiting jobs locally")
    parser.add_option("--test",     type="int", help="run test idle jobs")

    parser.add_option("--jid",      type="int", help=SUPPRESS_HELP) # help="job ID in database")
    parser.add_option("--setreturn",type="int", help=SUPPRESS_HELP) # help="set job return code")
    parser.add_option("--setstatus",type="int", help=SUPPRESS_HELP) # help="set job status code")

    options, args = parser.parse_args()

    qs = {"account":options.account, "queue":options.queue}
    global dbfile
    dbfile = options.db
    curs,conn = connect_JobsDB(dbfile)

    if options.jid:
        retries = 15
        while retries:
            try:
                set_job_status(curs, options.jid, options.setstatus, options.setreturn, walltime="auto")
                conn.commit()
                conn.close()
                return
            except:
                time.sleep(1)
                retries -= 1
        return

    cores_resource_id = get_resource_id(curs,"cores","number of cores")
    set_resource_limit(curs, cores_resource_id, options.limit)
    get_walltime_requested(curs, 0) # force creation of walltime limit category
    display_resource_use(curs)
    summarize_DB_runstatus(curs)

    if options.test: make_test_jobs(curs, options.test); conn.commit()
    if options.launch and not options.runlocal: update_and_launch_q(conn, options.account, options.queue, options.trickle)
    if options.status: update_qstatus(conn)
    if options.cancel: cancel_queued_jobs(conn)
    if options.clear: clear_completed(conn)
    if options.cycle: cycle_launcher(conn, options.trickle, options.runlocal, twait=options.cycle, account=options.account, qname=options.queue)
    elif options.runlocal: run_local(curs, options.trickle); conn.commit()

    if options.jobfile and options.walltime:
        jcmds = [l.strip() for l in open(options.jobfile,"r").readlines() if l[0]!='#']
        make_upload_jobs(conn.cursor(), options.jobfile, jcmds, [("walltime",options.walltime), ("cores",options.nodes)])
        conn.commit()

    if options.bundle and options.walltime:
        rebundle(conn.cursor(), options.bundle, os.environ["HOME"]+"/jobs/%s/"%options.bundle, options.walltime)
        conn.commit()

    conn.close()

if __name__ == "__main__": run_commandline()

