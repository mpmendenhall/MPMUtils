## @file JobManager.py job submission manager for cluster computing environments

import sqlite3
import subprocess
import os
from optparse import OptionParser

jobstates = {"Idle":2,"Starting":3,"Running":3,"Completed":4,"Removed":6}

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
        if n: print("\tStatus %i: %i jobs"%(i,n))

def checkjob(jid):
    """Get info from checkjob"""
    jdat = {}
    for j in subprocess.getoutput("checkjob %i"%jid).split("\n"):
        j = j.split(":")
        if len(j)==2: jdat[j[0].strip()] = j[1].strip()
    jdat["status"] = jobstates.get(jdat.get("State","unknown"),5)
    print(jdat)
    return jdat

def update_DB_runstatus(curs,jstatus):
    """Update run status in database from job status list"""
    curs.executemany("UPDATE jobs SET status=? WHERE queue_id=?",[(j[1],j[0]) for j in jstatus])
    
    jknown = frozenset([j[0] for j in jstatus if 1 <= j[1] <= 3])
    curs.execute("SELECT queue_id FROM jobs WHERE 1 <= status AND (status <= 3 OR status == 5)")
    missingjobs = [j[0] for j in curs.fetchall() if j[0] not in jknown]
    missingjobs = [(checkjob(j)["status"],j) for j in missingjobs]
    if missingjobs: curs.executemany("UPDATE jobs SET status=? WHERE queue_id=?",missingjobs)
    
def msub_job(curs, j, qsettings, mcmds=["-j oe", "-V"]):
    """Submit job via msub; update DB"""    
    assert j[1] == 0    # must be in waiting status
    
    # modify job script with msub commands
    jobin = open(j[4],"r").read()
    jobout = open(j[4],"w")
    
    jobout.write("#MSUB -A %s\n"%qsettings["account"])
    if j[5]: jobout.write("#MSUB -o %s\n"%j[5])
    if j[3]: jobout.write("#MSUB -N %s\n"%j[3])
    jobout.write("#MSUB -q %s\n"%qsettings["queue"])
    jobout.write("#MSUB -l nodes=%i\n"%j[7])
    jobout.write("#MSUB -l walltime=%i\n"%j[8])
    for m in mcmds: jobout.write("#MSUB "+m+"\n")
    
    jobout.write(jobin)
    jobout.close()
    
    o = subprocess.getoutput('msub "%s"'%j[4]).strip()
    print("Job '%s' submitted as '%s'"%(j[4],o))
    qid = int(o.split(".")[0])
    curs.execute("UPDATE jobs SET queue_id=?, status=1 WHERE job_id = ?",(qid,j[0]))

def get_nmore(curs,n):
    """Get next n processors worth of jobs waiting for submission"""
    jout = []
    curs.execute("SELECT * FROM jobs WHERE status = 0 LIMIT ?",(n,))
    for j in curs.fetchall():
        n -= j[7]
        if n<0: break
        jout.append(j)
    return jout

def update(qsettings):
    """Update database job status"""
    jstatus = get_showq_runstatus()
    conn = sqlite3.connect(qsettings["db"])
    curs = conn.cursor()
    conn.isolation_level = 'EXCLUSIVE'
    conn.execute('BEGIN EXCLUSIVE')
    update_DB_runstatus(curs,jstatus)
    conn.commit()
    conn.close() 
    
def update_and_launch(qsettings):
    """Update status; launch new jobs as available"""
    jstatus = get_showq_runstatus()
    ncpu = 0
    for j in jstatus:
        if j[1] <= 3: ncpu += j[2]
    print("Found reservations for %i cpu cores"%ncpu)
    
    conn = sqlite3.connect(qsettings["db"])
    curs = conn.cursor()
    
    conn.isolation_level = 'EXCLUSIVE'
    conn.execute('BEGIN EXCLUSIVE')

    update_DB_runstatus(curs,jstatus)
    if ncpu < qsettings["limit"]:
        jnext = get_nmore(curs,qsettings["limit"]-ncpu)
        print("Submitting %i new jobs."%len(jnext))
        for j in jnext: msub_job(curs,j,qsettings)
    
    conn.commit()
    
    summarize_DB_runstatus(curs)
    conn.close()

def cancel_queued_jobs(qsettings):
    """Force cancel all queued up jobs"""
    update(qsettings)
    
    conn = sqlite3.connect(qsettings["db"])
    curs = conn.cursor()
    curs.execute("SELECT queue_id FROM jobs WHERE queue_id > 0 AND status <= 2")
    for j in curs.fetchall():
        os.system("mjobctl -F %i"%j)
    curs.execute("UPDATE jobs SET status = 6 WHERE status = 0")
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





###################
###################
###################
if __name__ == "__main__":
    parser = OptionParser()
    
    parser.add_option("--account",  help="submission user account")
    parser.add_option("--queue",  help="submission queue")
    parser.add_option("--limit",  help="concurrent jobs limit")
    parser.add_option("--db",  help="jobs database")
    
    parser.add_option("--launch", action="store_true", help="update and launch")
    parser.add_option("--status", action="store_true", help="update and display status")
    parser.add_option("--cancel", action="store_true", help="cancel queued jobs")
    
    options, args = parser.parse_args()

    qs = {"account":options.account, "queue":options.queue, "limit":options.limit, "db":options.db}
    
    if options.launch: update_and_launch(qs)
    if options.status: update(qs)
    if options.cancel: cancel_queued_jobs(qs)

