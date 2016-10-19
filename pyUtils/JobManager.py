## @file JobManager.py job submission manager for cluster computing environments

import sqlite3
import subprocess
import os
from optparse import OptionParser
from time import sleep

jobstates = {"Idle":2,"Starting":3,"Running":3,"Completed":4,"Unknown":5,"Removed":6,"Bundled":7}
statenames = {-1:"hold", 0:"waiting", 1:"sumitted", 2:"queued", 3:"running", 4: "done" }
def statename(i): return statenames.get(i,"status %i"%i)

def get_showq_runstatus():
    """Get queue status according to showq"""
    cmd = 'showq -w user=$USER'
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

def summarize_DB_runstatus(curs):
    """Print summary counts by run status in DB"""
    print("Job submission database summary:")
    for i in range(11):
        curs.execute("SELECT COUNT(*) FROM jobs WHERE status=?",(i,))
        n = curs.fetchone()[0]
        if n: print("\t%s: %i jobs"%(statename(i),n))

def checkjob(jid):
    """Get info from checkjob"""
    jdat = {}
    for j in subprocess.getoutput("checkjob %i"%jid).split("\n"):
        j = j.split(":")
        if not j: continue
        if len(j)==2: jdat[j[0].strip()] = j[1].strip()
        elif j[0]=="Completion Code": jdat["ret_code"] = int(j[1].split()[0])
        elif j[0]=="WallTime": jdat["walltime"] = 3600*int(j[1]) + 60*int(j[2]) + int(j[3].split()[0])
    jdat["status"] = jobstates.get(jdat.get("State","unknown"),5)
    print(jdat)
    return jdat

def update_DB_runstatus(curs,jstatus):
    """Update run status in database from job status list"""
    curs.executemany("UPDATE jobs SET status=? WHERE queue_id=?",[(j[1],j[0]) for j in jstatus])

    jknown = frozenset([j[0] for j in jstatus if 1 <= j[1] <= 3])
    curs.execute("SELECT queue_id FROM jobs WHERE 1 <= status AND (status <= 3 OR status == 5)")
    missingjobs = [(j[0],checkjob(j[0])) for j in curs.fetchall() if j[0] not in jknown]
    missingjobs = [(j[1]["status"],j[1].get("ret_code",None),j[1].get("walltime",None),j[0]) for j in missingjobs]
    if missingjobs: curs.executemany("UPDATE jobs SET status=?,return_code=?,use_walltime=? WHERE queue_id=?",missingjobs)

def msub_job(curs, jid, qsettings, mcmds=["-j oe", "-V"]):
    """Submit job via msub; update DB"""

    curs.execute("SELECT name,jobfile,outlog,n_nodes,est_walltime FROM jobs WHERE job_id = ?",(jid,))
    j = curs.fetchone()
    if not j: return

    # modify job script with msub commands
    jobin = open(j[1],"r").read()
    jobout = open(j[1],"w")

    jobout.write("#MSUB -A %s\n"%qsettings["account"])
    if j[2]: jobout.write("#MSUB -o %s\n"%j[2])
    if j[0]: jobout.write("#MSUB -N %s\n"%j[0])
    jobout.write("#MSUB -q %s\n"%qsettings["queue"])
    jobout.write("#MSUB -l nodes=%i\n"%j[3])
    jobout.write("#MSUB -l walltime=%i\n"%j[4])
    for m in mcmds: jobout.write("#MSUB "+m+"\n")

    jobout.write(jobin)
    jobout.close()

    o = subprocess.getoutput('msub "%s"'%j[1]).strip()
    print("Job '%s' submitted as '%s'"%(j[0],o))
    qid = int(o.split(".")[0])
    curs.execute("UPDATE jobs SET queue_id=?, status=1 WHERE job_id = ?",(qid,jid))

def get_nmore(curs,n):
    """Get next n processors worth of jobs waiting for submission"""
    jout = []
    curs.execute("SELECT job_id,n_nodes FROM jobs WHERE status = 0 LIMIT ?",(n,))
    for j in curs.fetchall():
        n -= j[1]
        if n<0: break
        jout.append(j[0])
    return jout

def update(conn):
    """Update database job status"""
    jstatus = get_showq_runstatus()
    curs = conn.cursor()
    conn.isolation_level = 'EXCLUSIVE'
    conn.execute('BEGIN EXCLUSIVE')
    update_DB_runstatus(curs,jstatus)
    conn.commit()

def update_and_launch(conn,qsettings):
    """Update status; launch new jobs as available"""
    jstatus = get_showq_runstatus()
    ncpu = 0
    for j in jstatus:
        if j[1] <= 3: ncpu += j[2]
    print("Found reservations for %i cpu cores"%ncpu)

    curs = conn.cursor()

    conn.isolation_level = 'EXCLUSIVE'
    conn.execute('BEGIN EXCLUSIVE')

    update_DB_runstatus(curs,jstatus)
    conn.commit()
    if ncpu < qsettings["limit"]:
        jnext = get_nmore(curs,qsettings["limit"]-ncpu)
        print("Submitting %i new jobs."%len(jnext))
        for j in jnext:
            msub_job(curs,j,qsettings)
            conn.commit()

    summarize_DB_runstatus(curs)

def cancel_queued_jobs(conn):
    """Force cancel all queued up jobs"""
    update(conn)

    curs = conn.cursor()
    curs.execute("SELECT queue_id FROM jobs WHERE queue_id > 0 AND status <= 2")
    for j in curs.fetchall():
        os.system("mjobctl -F %i"%j)
    curs.execute("UPDATE jobs SET status = 6 WHERE status = 0")
    conn.commit()
    summarize_DB_runstatus(curs)


def load_test_batch(qsettings, njobs=8):
    """Generate test run batch"""
    home = os.environ["HOME"]
    for i in range(njobs):
        open(home+"/test_%i.sh"%i,"w").write('echo "Hello world %i!"\nsleep 30\necho "Goodbye!"'%i)

    conn = sqlite3.connect(qsettings["db"])
    curs = conn.cursor()
    with conn:
        upload_jobs(curs, [("test", home+"/test_%i.sh"%i, home+"/log_%i.txt"%i, 1, 60) for i in range(njobs)])
    conn.close()

def upload_onejob(curs, name, infile, logfile, nnodes, walltime):
    """Upload one job to run; return DB job identifier"""
    curs.execute("INSERT INTO jobs(name,jobfile,outlog,n_nodes,est_walltime,status) VALUES (?,?,?,?,?,0)", (name, infile, logfile, nnodes, walltime))
    return curs.lastrowid

def upload_jobs(curs,joblist):
    """Upload a list of jobs to run"""
    # job specifier: (name,input,log,nodes,walltime)
    print("Uploading jobs")
    for j in joblist: print("\t",j)
    curs.executemany("INSERT INTO jobs(name,jobfile,outlog,n_nodes,est_walltime,status) VALUES (?,?,?,?,?,0)", joblist)

def make_upload_jobs(curs, jname, jcmds, walltime, nodes=1):
    """Generate jobfiles for "one liners" and upload to DB"""
    # jcmds = (contents, logfile)
    jobdir = os.environ["HOME"]+"/jobs/%s/"%jname
    os.makedirs(jobdir, exist_ok=True)
    joblist = []
    for (n,jc) in enumerate(jcmds):
        jfl = jobdir + "/job_%i.sh"%n
        open(jfl,"w").write(jc[0])
        joblist.append((jname,jfl,jc[1],nodes,walltime))
    upload_jobs(curs,joblist)

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
            bjobfile.write("source %s > %s 2>&1\n"%(jlist[i][1], jlist[i][2]))
        bjobfile.close()
        bundle_id = upload_onejob(curs, "bundle_"+jname, bfname, bfname+"_log.txt", nnodes, tsum)
        for i in jb: curs.execute("UPDATE jobs SET status=7,associated=? WHERE job_id=?",(bundle_id,jlist[i][0]))

def rebundle(curs,jname,bundledir,tmax,nmax=1000):
    """Bundle small jobs together into longer-time units"""
    curs.execute("SELECT job_id,jobfile,outlog,n_nodes,est_walltime FROM jobs WHERE status=0 AND name=? AND est_walltime < ?",(jname,tmax))
    njobs = {}
    for r in curs.fetchall(): njobs.setdefault(r[3],[]).append(r)
    for n in njobs: make_bundle_jobs(curs, n, jname, bundledir, njobs[n], tmax, nmax)

def clear_completed(conn):
    curs = conn.cursor()
    curs.execute("PRAGMA foreign_keys = ON")
    curs.execute("DELETE FROM jobs WHERE status=4 OR status=6")
    conn.commit()

def cycle_launcher(conn,qsettings,twait=60):
    while 1:
        update_and_launch(conn,qsettings)
        sleep(twait)

###################
###################
###################
if __name__ == "__main__":
    parser = OptionParser()

    parser.add_option("--account",  help="submission billing account")
    parser.add_option("--queue",  help="submission queue")
    parser.add_option("--limit",  type="int", default=10, help="concurrent jobs limit")
    parser.add_option("--db",  help="jobs database")

    parser.add_option("--launch", action="store_true", help="update and launch")
    parser.add_option("--cycle", action="store_true", help="continuously re-check jobs")
    parser.add_option("--status", action="store_true", help="update and display status")
    parser.add_option("--cancel", action="store_true", help="cancel queued jobs")
    parser.add_option("--clear", action="store_true", help="clear completed jobs")
    parser.add_option("--jobfile", help="run one-liners in file")
    parser.add_option("--walltime", type="int", help="wall time for 1-liner jobs in seconds")
    parser.add_option("--nodes", type="int", default=1, help="nodes for 1-liner jobs")

    options, args = parser.parse_args()

    qs = {"account":options.account, "queue":options.queue, "limit":options.limit, "db":options.db}
    conn = sqlite3.connect(options.db)

    if options.launch: update_and_launch(conn,qs)
    if options.cycle: cycle_launcher(conn,qs)
    if options.status: update(conn)
    if options.cancel: cancel_queued_jobs(conn)
    if options.clear: clear_completed(conn)
    if options.cycle: cycle_launcher(conn,qs)
    if options.jobfile and options.walltime:
        jcmds = [l.strip() for l in open(options.jobfile,"r").readlines() if l[0]!='#']
        make_upload_jobs(conn.cursor(), options.jobfile, jcmds, options.walltime, options.nodes)
        conn.commit()

    conn.close()

