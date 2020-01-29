#! /bin/env python3
## @file JobManager_DB.py JobsDB interface
# Michael P. Mendenhall, LLNL 2019

import sqlite3
import time
import os
import shlex
import bisect
import heapq

# Job state identifiers and corresponding names
statenames = {-1:"hold", 0:"waiting", 1:"sumitted", 2:"queued", 3:"running", 4: "done", 5:"???", 6:"removed", 7:"bundled", 8:"unbundled" }
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
        try: os.remove(logfile)
        except: pass
        return self.jid

    def __str__(self):
        return "Job " + self.name + " [" + self.q + "," + str(self.acct) + "]"

def connect_JobsDB(fname, remake = True):
    """Get connection to Jobs DB"""

    global dbfile
    if fname is None: fname = dbfile
    remake &= not os.path.exists(fname)
    if remake:
        print("Initializing new JobsDB at", fname)
        os.makedirs(os.path.dirname(fname), exist_ok=True)
        os.system("sqlite3 " + fname + " < " + os.path.dirname(os.path.abspath(__file__)) + "/JobsDB_Schema.sql")
    conn = sqlite3.connect(fname, timeout = 60, isolation_level = "DEFERRED")
    conn.cursor().execute("PRAGMA foreign_keys = ON")
    if remake: conn.cursor().execute("PRAGMA journal_mode = WAL")
    dbfile = fname
    return conn

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

def clear_job(curs, jid):
    """Remove job entry from DB, and associated logfile"""
    curs.execute("SELECT outlog FROM jobs WHERE job_id = ?", (jid,))
    try: os.remove(curs.fetchone()[0])
    except: pass
    curs.execute("DELETE FROM jobs WHERE job_id = ?", (jid,))

def clear_completed(conn, clearlogs = True):
    curs = conn.cursor()
    if clearlogs:
        curs.execute("SELECT outlog FROM jobs WHERE status=4 OR status=6")
        for r in curs.fetchall():
            try: os.remove(r[0])
            except: pass
    curs.execute("DELETE FROM jobs WHERE status=4 OR status=6")
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

def get_possible_submissions(curs, nmax = 10000, lifo = True):
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

def combined_time(jtimes, nnodes):
    """Estimate combined run times for serial/parallel jobs"""
    if nnodes < 2: return sum(jtimes)
    if len(jtimes) <= nnodes: return max(jtimes)

    h = [0.]*nnodes
    for j in jtimes: heapq.heappush(h, heappop(h) + j)
    return max(h)

def create_bundle(curs, jids, tbundle, bname, blog, nnodes):
    """Create combined bundle of [job_id, ...]"""

    J = get_job_info(curs, jids[0])
    J.status = 0
    J.name = bname
    J.logfile = blog
    J.script = os.path.dirname(os.path.abspath(__file__)) + '/JobManager_Worker.py --db ' + shlex.quote(dbfile)
    if nnodes is not None: J.script += " --n %i "%nnodes
    J.script += " --tmax %g"%tbundle + " --jlist " + ",".join(str(j) for j in jids)
    J.res_list = [("walltime", tbundle), ("cores", nnodes)]

    print(J.script)
    J.upload(curs)

    curs.executemany("UPDATE jobs SET status=7, associated=? WHERE job_id=?",[(J.jid, j) for j in jids])

def choose_bundles(ts, tmax, nslots):
    """Determine how to pack items [(t, id),...] into bundles with n slots, max tmax"""
    ts.sort()
    bout = []
    bnew = []
    h = [0.]*nslots

    while len(ts):
        m = heapq.heappop(h) # least-used slot
        i = bisect.bisect_left(ts, (tmax - m,))
        if not i: # no room left?
            if not bnew: break # nothing fits any bundles
            bout.append(bnew)
            bnew = []
            h = [0.]*nslots
        heapq.heappush(h, m + ts[i-1][0])
        bnew.append(ts.pop(i-1))

    if bnew: bout.append(bnew)
    return bout

def rebundle(curs, jname, tmax, nslots):
    """Bundle small jobs together into longer-time units"""

    curs.execute("SELECT quantity, job_id FROM jobs NATURAL JOIN resource_use \
        WHERE (status = 0 OR status = 8) AND name = ? AND resource_id = ? AND quantity < ?", (jname, find_resource_id(curs, "walltime"), tmax))
    js = list(curs.fetchall())
    if not js: return

    logdir = jobscript_dir + "/" + jname
    os.makedirs(logdir, exist_ok=True)

    njobs = len(js)
    bundles = choose_bundles(js, tmax, nslots)
    print("Bundling %i jobs into %i (%g s)x(%i slot) groups..."%(njobs, len(bundles), tmax, nslots))
    for jb in bundles:
        create_bundle(curs, [j[1] for j in jb], tmax, "bundled_"+jname,
                        logdir + "/bundle_log_%i.txt"%jb[0][1], nslots)

def post_completion(curs, jid):
    """Post-completion wrap-up"""

    # bundled jobs check
    curs.execute("SELECT job_id FROM jobs WHERE associated = ?", (jid,))
    bundled = [r[0] for r in curs.fetchall()]
    if not bundled: return
    print("Unbundling", len(bundled), "jobs.")

    # read bundle output log
    curs.execute("SELECT outlog FROM jobs WHERE job_id = ?", (jid,))
    try: ls = open(curs.fetchone()[0], "r").readlines()
    except: ls = []
    newstats = []
    for l in ls:
        l = l.split()
        if l[0] != "BundleJob" or len(l) != 5: continue
        newstats.append((int(l[2]), float(l[3]), float(l[4]), int(l[1])))
    curs.executemany("UPDATE jobs SET status = 4, return_code = ?, t_submit = ?, use_walltime = ?, associated = Null WHERE job_id = ?", newstats)

    # completion of successfully unbundled jobs...
    jdone = [n[-1] for n in newstats]
    for j in jdone: post_completion(curs, j)

    # bundle jobs missing results?
    incomplete = [(j,) for j in bundled if j not in jdone]
    if len(incomplete):
        print(len(incomplete), "bundled jobs uncompleted!")
        curs.executemany("UPDATE jobs SET status = 8, associated = Null WHERE job_id = ?", incomplete)
    else: clear_job(curs, jid)
