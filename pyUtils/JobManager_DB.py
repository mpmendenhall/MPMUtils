#! /bin/env python3
## @file JobManager_DB.py JobsDB interface
# Michael P. Mendenhall, LLNL 2019

import sqlite3
import time
import os

# Job state identifiers and corresponding names
statenames = {-1:"hold", 0:"waiting", 1:"sumitted", 2:"queued", 3:"running", 4: "done", 5:"???", 6:"removed", 7:"bundled" }
def statename(i): return statenames.get(i,"status %i"%i)

# directory for job scripts
jobscript_dir = os.environ["HOME"]+"/jobs/"
# path to database file
dbfile = jobscript_dir+"/jdb.sql"
# cached resource IDs by name
resource_ids = { }

class Job:
    """Description of a job to run"""

    def __init__(self, name=None, q=None, acct=None, script=None, logfile=None, jid=None, qid=None, res_list=[]):
        self.name = name            # job name
        self.q = q                  # submission queue
        self.acct = acct            # submission account
        self.script = script        # job script file
        self.logfile = logfile      # output logfile
        self.res_list = res_list    # resource requirements
        self.jid = jid              # JobsDB unique identifier
        self.qid = qid              # Queue system unique identifier

    def upload(self, curs):
        """upload to JobsDB"""
        curs.execute("INSERT INTO jobs(name,q_name,q_acct,jobscript,outlog,status) VALUES (?,?,?,?,?,0)",
                     (self.name, self.q, self.acct, self.script, self.logfile))
        self.jid = curs.lastrowid
        for rs in self.res_list: set_job_resource(curs, self.jid, rs[0], rs[1])
        return self.jid

    def __str__(self):
        return "Job " + self.name + " [" + self.q + "," + str(self.acct) + "]"

def connect_JobsDB(fname, remake = True):
    """Get connection to Jobs DB"""

    if remake and not os.path.exists(fname):
        print("Initializing new JobsDB at", fname)
        os.makedirs(os.path.dirname(fname), exist_ok=True)
        os.system("sqlite3 " + fname + " < " + os.path.dirname(os.path.abspath(__file__)) + "/JobsDB_Schema.sql")
    conn = sqlite3.connect(fname, timeout = 60, isolation_level = "DEFERRED")
    conn.cursor().execute("PRAGMA foreign_keys = ON")
    conn.cursor().execute("PRAGMA journal_mode = WAL")
    global dbfile
    dbfile = fname
    return conn

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
        print(f"\t{rs[1]} [{rs[2]}]: {resuse[1]} / {resuse[0]} used")
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

###########
###########

def get_job_info(curs, jid):
    """Get basic information on job"""
    curs.execute("SELECT status, queue_id, name, q_name, q_acct, jobscript, outlog FROM jobs WHERE job_id = ?", (jid,))
    j = curs.fetchone()
    if not j: return

    J = Job(jid=jid, qid=j[1], name=j[2], q=j[3], acct=j[4], script=j[5], logfile=j[6])
    J.wtreq = get_walltime_requested(curs, jid)
    if J.wtreq is None: J.wtreq = 1800
    J.ncores = get_cores_requested(curs, jid)
    if J.ncores is None: J.ncores = 1

    return J

def make_upload_jobs(curs, jname, q, acct, jcmds, res_use):
    """Generate jobfiles for "one liners" and upload to DB"""

    jobdir = jobscript_dir + "/" + jname
    os.makedirs(jobdir, exist_ok=True)

    # look up named resources to DB IDs
    for rs in res_use: rs = (get_resource_id(curs,rs[0],rs[0]), rs[1])

    return [Job(name=jname, q=q, acct=acct, script=jc, res_list=res_use, logfile=jobdir+"/log_%i.txt"%n).upload(curs) for (n,jc) in enumerate(jcmds)]

def make_test_jobs(curs, njobs, q, acct):
    """Generate test run batch"""
    jcmds = ['echo "Hello world %i!"\nsleep 5\necho "Goodbye!"'%i + ("\nexit -99" if not (i+1)%5 else "") for i in range(njobs)]
    make_upload_jobs(curs, "test", q, acct, jcmds, [("walltime", 300), ("local_cores" if q == 'local' else "cores",1)])

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
