#! /bin/env python3
## @file JobManager.py job submission manager for cluster computing environments

import sqlite3
import subprocess
import os
import sys
from argparse import ArgumentParser, SUPPRESS
import time
import multiprocessing
import datetime

jobstates = {"Idle":2, "ReqNodeNotA":2, "Starting":3, "Running":3, "Completed":4, "Unknown":5, "Removed":6, "Bundled":7}
statenames = {-1:"hold", 0:"waiting", 1:"sumitted", 2:"queued", 3:"running", 4: "done", 6:"removed", 7:"bundled" }
def statename(i): return statenames.get(i,"status %i"%i)
jobscript_dir = os.environ["HOME"]+"/jobs/"
dbfile = jobscript_dir+"/jdb.sql" # path to database file
resource_ids = { } # cached resource IDs by name

def connect_JobsDB(fname, remake = True):
    """Get cursor and connection for Jobs DB"""
    if remake and not os.path.exists(fname):
        print("Initializing new JobsDB at", fname)
        os.makedirs(os.path.dirname(fname), exist_ok=True)
        os.system("sqlite3 %s < %s/pyUtils/JobsDB_Schema.sql"%(fname, os.environ["MPMUTILS"]))
    conn = sqlite3.connect(fname, timeout = 60, isolation_level = "DEFERRED")
    curs = conn.cursor()
    curs.execute("PRAGMA foreign_keys = ON")
    curs.execute("PRAGMA journal_mode = WAL")
    global dbfile
    dbfile = fname
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
        curs.execute("SELECT outlog FROM jobs WHERE status=4 OR status=6 OR status=7")
        for r in curs.fetchall():
            if(r[0]): os.system("rm -f "+r[0])
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

def create_resource(curs, name, descrip, lim = 1):
    """Create named resource; return id"""
    curs.execute("INSERT INTO resources(name,descrip,available) VALUES (?,?,?)", (name, descrip, lim))
    resource_ids[name] = curs.lastrowid
    return resource_ids[name]

def get_resource_id(curs, name, descrip, lim = 1):
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

def get_possible_submissions(curs, nmax = 100000, lifo = True):
    """Select list of waiting (jobs, resources) that could be submitted within resource limits"""
    jout = []
    resuse = {}
    if lifo: curs.execute("SELECT job_id FROM jobs WHERE status = 0 ORDER BY -job_id LIMIT ?",(nmax,))
    else: curs.execute("SELECT job_id FROM jobs WHERE status = 0 LIMIT ?",(nmax,))
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
        if var == "State":
            jdat[var] = val
            jdat["ret_code"] = 0 if val=='Completed' else -1
        elif var=="WallTime":
            val = val.split(":")
            try: jdat["walltime"] = 3600*int(val[0]) + 60*int(val[1]) + int(val[2].split()[0])
            except: jdat["walltime"] = None
        else: jdat[var]=val
    if "StartTime" in jdat:
        Y = datetime.datetime.strftime(datetime.datetime.now(), "%Y ")
        try: jdat["xStartTime"] = datetime.datetime.strptime(Y + jdat["StartTime"], "%Y %a %b %d %H:%M:%S")
        except: pass
    if jdat.get("Walltime", None) == None:
        if "xStartTime" in jdat: jdat["walltime"] = (datetime.datetime.now() - jdat["xStartTime"]).total_seconds()
    jdat["status"] = jobstates.get(jdat.get("State","unknown"),5)
    print(jdat)
    return jdat

def msub_job(curs, jid, mcmds=["-j oe", "-V"]):
    """Submit job via msub; update DB"""

    curs.execute("SELECT name,jobscript,outlog,q_name,q_acct FROM jobs WHERE job_id = ?",(jid,))
    j = curs.fetchone()
    if not j: return

    wtreq = get_walltime_requested(curs,jid)
    if wtreq is None: wtreq = 1800
    ncores = get_cores_requested(curs,jid)
    if ncores is None: ncores = 1

    cmd = ["msub", "-l", "nodes=%i"%ncores, "-l", "walltime=%i"%wtreq]
    if j[0]: cmd += ["-N", "%s_%i"%(j[0],jid)]
    if j[2]: cmd += ["-o", j[2]]
    if j[3]: cmd += ["-q", j[3]]
    if j[4]: cmd += ["-A", j[4]]

    msub = subprocess.Popen(cmd, stdout = subprocess.PIPE, stdin = subprocess.PIPE, stderr = subprocess.PIPE)
    msub.stdin.write('\n'.join(["#MSUB "+m for m in mcmds]+[j[1],]).encode())
    ou,er = msub.communicate()

    o = ou.decode().strip()
    print("Job '%s' submitted as '%s'"%(j[0],o))
    qid = int(o.split()[-1].split(".")[0])
    curs.execute("UPDATE jobs SET queue_id=?, status=1, t_submit = ? WHERE job_id = ?",(qid,time.time(),jid))

def update_DB_qrunstatus(curs,jstatus):
    """Update run status in database from job status list"""
    curs.executemany("UPDATE jobs SET status=? WHERE queue_id=?",[(j[1],j[0]) for j in jstatus])

    jknown = frozenset([j[0] for j in jstatus if 1 <= j[1] <= 3])
    curs.execute("SELECT queue_id FROM jobs WHERE 1 <= status AND (status <= 3 OR status == 5) AND (q_name is NULL OR q_name != 'local')")
    missingjobs = [(j[0],checkjob(j[0])) for j in curs.fetchall() if j[0] not in jknown]
    missingjobs = [(j[1]["status"],j[1].get("ret_code",None),j[1].get("walltime",None),j[0]) for j in missingjobs]
    if missingjobs: curs.executemany("UPDATE jobs SET status=?,return_code=?,use_walltime=? WHERE queue_id=?",missingjobs)

def check_running_qjobs(conn):
    """Check (formerly) running jobs; update DB status appropriately"""
    jstatus = get_showq_runstatus()
    curs = conn.cursor()
    update_DB_qrunstatus(curs,jstatus)
    conn.commit()

def update_qstatus(conn):
    """Update database job status"""
    jstatus = get_showq_runstatus()
    curs = conn.cursor()
    update_DB_qrunstatus(curs,jstatus)
    conn.commit()

def launch_local(curs, jobid):
    """Background-launch a job locally"""
    assert False # TODO
    curs.execute("SELECT jobscript,outlog FROM jobs WHERE job_id = ?", (jobid,))
    r = curs.fetchone()
    jcmd = r[0]
    if r[1]: jcmd += " > %s 2>&1"%r[1]
    jcmd += "; python3 %s --db %s --jid %i --setreturn $? --setstatus 4"%(__file__, dbfile, jobid)
    print(jcmd, flush=True)
    curs.execute("UPDATE jobs SET status = 1, t_submit = ? WHERE job_id = ?", (time.time(), jobid))
    pid = subprocess.Popen([jcmd,], shell=True, preexec_fn=(lambda : os.nice(15))).pid
    curs.execute("UPDATE jobs SET status = 3, queue_id = ? WHERE job_id = ? AND status != 4", (pid, jobid))

def check_running_localjobs(curs):
    """Check (formerly) running jobs; update DB status appropriately"""
    curs.execute("SELECT job_id,queue_id FROM jobs WHERE status = 3 AND q_name = 'local'")
    for j in curs.fetchall():
        try: os.kill(j[1], 0)
        except:
            print("Job",j[0],j[1],"failed to report back!")
            curs.execute("UPDATE jobs SET status = 4, return_code = -999 WHERE job_id = ? AND status = 3", (j[0],))

def update_and_launch(conn, curs, trickle = None):
    """Update status; launch new jobs as available"""

    jstatus = get_showq_runstatus()
    update_DB_qrunstatus(curs,jstatus)
    check_running_localjobs(curs)
    conn.commit()

    jnext = get_possible_submissions(curs)
    if len(jnext):
        print("Submitting %i new jobs."%len(jnext))
        t0 = datetime.datetime.now() if trickle else None
        for j in jnext:
            curs.execute("SELECT q_name FROM jobs WHERE job_id = ?", (j[0],))
            if curs.fetchone()[0] == "local":
                launch_local(curs,j[0])
                if trickle: time.sleep(trickle)
            else:
                mcmds=["-j oe", "-V"]
                if t0:
                    t0 += datetime.timedelta(seconds=trickle)
                    mcmds.append("-a "+t0.strftime("%Y%m%d%H%M.%S"))
                msub_job(curs, j[0], mcmds)
            conn.commit()

    summarize_DB_runstatus(curs)

def cancel_queued_jobs(conn):
    """Force cancel all queued up jobs"""
    update_qstatus(conn)

    curs = conn.cursor()
    curs.execute("SELECT queue_id FROM jobs WHERE queue_id > 0 AND status <= 2")
    for j in curs.fetchall(): os.system("mjobctl -c %i"%j)
    curs.execute("UPDATE jobs SET status = 6 WHERE status = 0")
    conn.commit()
    summarize_DB_runstatus(curs)


def kill_jobs(conn, usr, account):
    os.system("scancel -u %s -A %s"%(usr, account))
    update_qstatus(conn)

##
## job launching
##

class Job:
    """Description of a job to run"""
    def __init__(self, name=None, q=None, acct=None, script=None, logfile=None, res_list=[]):
        self.name = name
        self.q = q
        self.acct = acct
        self.script = script
        self.logfile = logfile
        self.res_list = res_list
        self.jid = None

    def upload(self, curs):
        curs.execute("INSERT INTO jobs(name,q_name,q_acct,jobscript,outlog,status) VALUES (?,?,?,?,?,0)",
                     (self.name, self.q, self.acct, self.script, self.logfile))
        self.jid = curs.lastrowid
        for rs in self.res_list: set_job_resource(curs, self.jid, rs[0], rs[1])
        return self.jid

    def __str__(self):
        return "Job " + self.name + " [" + self.q + "," + str(self.acct) + "]"

def make_upload_jobs(curs, jname, q, acct, jcmds, res_use):
    """Generate jobfiles for "one liners" and upload to DB"""

    jobdir = jobscript_dir + "/" + jname
    os.makedirs(jobdir, exist_ok=True)

    # look up named resources to DB IDs
    for rs in res_use: rs = (get_resource_id(curs,rs[0],rs[0]), rs[1])

    return [Job(name=jname, q=q, acct=acct, script=jc, res_list=res_use, logfile=jobdir+"/log_%i.txt"%n).upload(curs) for (n,jc) in enumerate(jcmds)]

def make_test_jobs(curs, njobs, q, acct):
    """Generate test run batch"""
    jcmds = ['echo "Hello world %i!"\nsleep 5\necho "Goodbye!"'%i for i in range(njobs)]
    make_upload_jobs(curs, "test", q, acct, jcmds, [("walltime",30), ("local_cores" if q == 'local' else "cores",1)])

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
        tsum = 0
        bcmd = ""
        for i in jb:
            tsum += jlist[i][4]
            bcmd += jlist[i][1]
            if jlist[i][2]: bcmd += " > %s 2>&1"%jlist[i][2]
            bcmd+="\n"
        bundle_id = upload_onejob(curs, "bundle_"+jname, bcmd, bfname+"_log.txt", nnodes, tsum)
        for i in jb: curs.execute("UPDATE jobs SET status=7,associated=? WHERE job_id=?",(bundle_id,jlist[i][0]))

def rebundle(curs,jname,bundledir,tmax,nmax=1000):
    """Bundle small jobs together into longer-time units"""
    assert False # TODO est_walltime
    curs.execute("SELECT job_id,jobscript,outlog,n_nodes,est_walltime FROM jobs WHERE status=0 AND name=? AND est_walltime < ?",(jname,tmax))
    njobs = {}
    for r in curs.fetchall(): njobs.setdefault(r[3],[]).append(r)
    for n in njobs: make_bundle_jobs(curs, n, jname, bundledir, njobs[n], tmax, nmax)

def cycle_launcher(conn, trickle=0, twait=15):
    curs = conn.cursor()
    while 1:
        curs.execute("SELECT COUNT(*) FROM jobs WHERE status IN (0,1,2,3)")
        nleft = curs.fetchone()[0]
        print(nleft, "jobs uncompleted.", flush=True)
        if not nleft: break
        update_and_launch(conn, curs, trickle)
        conn.commit()
        time.sleep(twait)
    conn.commit()

###################
###################
###################

def run_commandline():
    parser = ArgumentParser()

    parser.add_argument("--account",  help="submission billing account")
    parser.add_argument("--queue",    help="submission queue --- set to 'local' for local run")
    parser.add_argument("--limit",    type=int,   help="concurrent jobs limit")
    parser.add_argument("--trickle",  type=float, help="time delay [s] between nominal run starts")
    parser.add_argument("--db",       help="jobs database")

    parser.add_argument("--launch",   action="store_true", help="update and launch")
    parser.add_argument("--cycle",    type=float, help="continuously re-check/launch jobs at specified interval")
    parser.add_argument("--status",   action="store_true", help="update and display status")
    parser.add_argument("--cancel",   action="store_true", help="cancel queued jobs")
    parser.add_argument("--kill",     action="store_true", help="kill running jobs")
    parser.add_argument("--clear",    action="store_true", help="clear completed jobs")
    parser.add_argument("--jobfile",  help="run one-liners in file")
    parser.add_argument("--name",     help="name for job(s)")
    parser.add_argument("--script",   action="store_true", help="supply script on stdin")
    parser.add_argument("--walltime", type=int, help="wall time for 1-liner jobs in seconds", default = 1800)
    parser.add_argument("--nodes",    type=int, default=1, help="nodes for 1-liner jobs")
    parser.add_argument("--bundle",   help="bundle job name; specify bundled walltime")
    parser.add_argument("--test",     type=int, help="run test idle jobs")

    # Special commands to allow locally-running jobs to report back on completion
    parser.add_argument("--jid",      type=int, help=SUPPRESS) # help="job ID in database")
    parser.add_argument("--setreturn",type=int, help=SUPPRESS) # help="set job return code")
    parser.add_argument("--setstatus",type=int, help=SUPPRESS) # help="set job status code")

    parser.add_argument("--hold",     action="store_true",    help="place all waiting jobs on hold status")
    parser.add_argument("--unhold",   action="store_true",    help="move all held jobs to waiting")

    options = parser.parse_args()

    global dbfile
    if options.db: dbfile = options.db
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

    if options.hold:
        curs.execute("UPDATE jobs SET status = -1 WHERE status = 0")
        conn.commit()
        return
    if options.unhold:
        curs.execute("UPDATE jobs SET status = 0 WHERE status = -1")
        conn.commit()
        return

    if options.limit:
        cores_resource_id = get_resource_id(curs,"cores","number of cores")
        set_resource_limit(curs, cores_resource_id, options.limit)
    display_resource_use(curs)
    summarize_DB_runstatus(curs)

    rsrc = [("walltime", options.walltime), ("local_cores" if options.queue == "local" else "cores", options.nodes)]
    jcmds = []
    jname = options.name
    if options.jobfile:
        if not jname: jname = options.jobfile
        jcmds = [l.strip() for l in open(options.jobfile,"r").readlines() if l[0]!='#']
    if options.script:
        print("Input job script on stdin; end with ctrl-D:")
        cmd = sys.stdin.read().strip()
        if cmd: jcmds.append(cmd)
    if jcmds:
        make_upload_jobs(conn.cursor(), jname if jname else 'anon', options.queue, options.account, jcmds, rsrc)
        conn.commit()

    if options.test: make_test_jobs(curs, options.test, options.queue, options.account); conn.commit()
    if options.launch: update_and_launch(conn, curs, options.trickle)
    if options.status: update_qstatus(conn)
    if options.cancel: cancel_queued_jobs(conn)
    if options.kill: kill_jobs(conn, os.environ["USER"], options.account)
    if options.clear: clear_completed(conn)
    if options.cycle: cycle_launcher(conn, options.trickle, twait=options.cycle)

    if options.bundle and options.walltime:
        rebundle(conn.cursor(), options.bundle, os.environ["HOME"]+"/jobs/%s/"%options.bundle, options.walltime)
        conn.commit()

    conn.close()

"""
./JobManager.py --test 10 --limit 5 --cycle 5
"""

if __name__ == "__main__": run_commandline()

