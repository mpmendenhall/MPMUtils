#! /bin/env python3
## @file JobManager_DB.py JobsDB interface
# Michael P. Mendenhall, LLNL 2020

import sqlite3
import time
import os
import shlex
import bisect
import heapq

# Job state identifiers and corresponding names
statenames = {-1:"hold", 0:"waiting", 1:"submitted", 2:"queued", 3:"running", 4: "done", 5:"???", 6:"removed", 7:"bundled", 8:"unbundled" }
def statename(i): return statenames.get(i,"status %i"%i)

# directory for job scripts
jobscript_dir = os.environ["HOME"]+"/jobs/"
# path to database file
dbfile = jobscript_dir+"/jdb.sql"
# cached resource IDs by name
resource_ids = { }


######################
######################

class Job:
    """Description of a shell-script job to run"""

    def __init__(self, name="anon", q=None, acct=None, script="", jid=None, qid=None, res_list = {}):
        self.name = name            # job name
        self.q = q                  # submission queue
        self.acct = acct            # submission account
        self.script = script        # job script
        self.res_list = res_list    # resource requirements
        self.jid = jid              # JobsDB unique identifier
        self.qid = qid              # Queue system unique identifier

    def upload(self, curs):
        """upload to JobsDB"""
        curs.execute("INSERT INTO jobs(jtype, name, q_name, q_acct, jobscript, status) VALUES (?,?,?,?,?,0)",
                     (self.jtype, self.name, self.q, self.acct, self.script))
        self.jid = curs.lastrowid
        for rs in self.res_list.items(): set_job_resource(curs, self.jid, rs[0], rs[1])
        return self.jid

    def getpath(self):
        """path to job info directory"""
        return jobscript_dir + "/" + (self.name if self.name else "anon") + "/%i/"%self.jid

    def prerun_setup(self, curs):
        """Set up necessary scripts, files before run"""
        os.makedirs(self.getpath(), exist_ok = True)

    def check_done(self, curs):
        """check job completion and update DB"""

        p = self.getpath()
        if not os.path.exists(p + "/exit.txt"): return False

        s = int(open(p + "/start.txt", "r").read().split()[0])
        e = open(p + "/exit.txt", "r").read().split()
        curs.execute("UPDATE jobs SET status = 4, return_code = ?, t_submit = ?, use_walltime = ?, associated = NULL WHERE job_id = ?",
                    (int(e[0]), s, int(e[1])-s, self.jid))

        return True

    def prerun_script(self):
        """script before main run"""
        return 'echo `date +"%%s"` $HOSTNAME > %s/start.txt\n'%self.getpath()

    def make_wrapper(self):
        """generate wrapper shell script"""
        d = self.getpath()
        open(d + "/wrapper.sh", "w").write("#!/bin/bash\n" + self.prerun_script()
                                           + "\n" + " ".join(self.get_run_command())
                                           + "\n" + self.postrun_script())
        os.chmod(d + "/wrapper.sh", 0o744)

    def postrun_script(self):
        """script after main run"""
        s  = 'excd=$?\n'
        s += 'echo $excd `date +"%%s"` > %s/exit.txt\n'%self.getpath()
        s += 'exit $excd\n'
        return s

    def __str__(self):
        return "Job " + self.name + ": %s"%str(self.jid) + " [" + str(self.q) + "," + str(self.acct) + "] " + str(self.res_list)

class ShellJob(Job):
    """Job defined by shell script"""

    def __init__(self, **kwargs):
        Job.__init__(self, **kwargs)
        self.jtype = 0

    def prerun_setup(self, curs):
        super().prerun_setup(curs)
        d = self.getpath()
        open(d + "/script.sh", "w").write(self.script + "\n")
        os.chmod(d + "/script.sh", 0o744)

    def get_run_command(self):
        """Get command to run to execute job"""
        return ["bash", self.getpath() + "/script.sh"]


class BundleJob(Job):
    """Serial/parallel bundle of jobs"""

    def __init__(self, **kwargs):
        Job.__init__(self, **kwargs)
        self.jtype = 1
        self.runorder = [int(j) for j in self.script.split()]

    def linear_order(self, ts, nslots, tmax):
        """Choose linear job ordering to efficiently fill available slots in parallel
        extracts from input sorted ts = [(t, id),...]"""

        h = [0.]*nslots      # times used in each slot (heap format)
        self.runorder = []   # job run order to fill bundle
        while len(ts):
            m = heapq.heappop(h) # contents of least-used slot (next available to run in)
            i = bisect.bisect_left(ts, (tmax + 0.01 - m,)) # biggest item that will fit
            if not i: # will not fit in least-used slot?
                heapq.heappush(h, m)
                break
            heapq.heappush(h, m + ts[i-1][0]) # fill into slot
            self.runorder.append(ts.pop(i-1)[1]) # append to ordering

        self.script = " ".join([str(r) for r in self.runorder])
        self.res_list["walltime"] = max(h)
        self.res_list["cores"] = min(nslots, len(self.runorder))

    def prerun_setup(self, curs):
        """Set up necessary scripts/paths before running"""
        super().prerun_setup(curs)
        s = ""
        for J in [get_job(curs, r) for r in self.runorder]:
            J.prerun_setup(curs)
            J.make_wrapper()
            d = J.getpath()
            s += "bash " + d + "/wrapper.sh > " + d + "/log.txt 2>&1\n"
        open(self.getpath() + "/jobs.txt", "w").write(s)

    def get_run_command(self):
        """Get command to run to execute job"""
        nc = int(self.res_list["cores"])
        if nc == 1: return ["bash", self.getpath() + "/jobs.txt"]
        return ["parallel", "-j", str(nc), "::::", self.getpath() + "/jobs.txt"]

    def check_done(self, curs):
        """check job completion and update DB"""

        if not super().check_done(curs): return False

        for jj in self.runorder:
            if not get_job(curs, jj).check_done(curs):
                print("Bundled sub-job did not cleanly exit:", jj)
                curs.execute("UPDATE jobs SET status = 5 WHERE job_id = ?", (jj,))

        return True

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
    #if remake: conn.cursor().execute("PRAGMA journal_mode = WAL")
    dbfile = fname
    return conn

def get_job(curs, jid):
    """Retrieve job object by database ID"""
    curs.execute("SELECT status, queue_id, name, q_name, q_acct, jobscript, jtype FROM jobs WHERE job_id = ?", (jid,))
    j = curs.fetchone()
    assert j and j[-1] in (0,1)

    if j[-1] == 0: J = ShellJob()
    elif j[-1] == 1:
        J = BundleJob()
        J.runorder = [int(j) for j in j[5].split()]

    J.jid   = jid
    J.qid   = j[1]
    J.name  = j[2]
    J.q     = j[3]
    J.acct  = j[4]
    J.script= j[5]
    J.res_list = dict([(r[1], r[2]) for r in get_job_resources(curs, jid)])

    return J

######################
######################

def summarize_DB_runstatus(curs):
    """Print summary counts by run status in DB"""
    print("Job submission database summary:")
    for i in range(11):
        curs.execute("SELECT COUNT(*) FROM jobs WHERE status=?",(i,))
        n = curs.fetchone()[0]
        if n: print("\t%s: %i jobs"%(statename(i),n))

def completed_runstats(curs, name):
    """Display stats on completed runs by grouping name"""

    curs.execute("SELECT use_walltime FROM jobs WHERE name LIKE ? AND status=4", (name,))
    rt = [r[0] for r in curs.fetchall()]
    n = len(rt)
    if not n:
        print("No completed jobs found named", name)
        return
    rt.sort()
    print("----- Completion time quantiles for %i jobs '%s'"%(n, name))
    print("0\t",    rt[0])
    print("25\t",   rt[n//4])
    print("50\t",   rt[n//2])
    print("75\t",   rt[(3*n)//4])
    if n >= 100:  print("95\t",   rt[(95*n)//100])
    if n >= 1000: print("99\t",   rt[(99*n)//100])
    print("100\t",  rt[-1])
    print("Mean", sum(rt)/n);

def clear_completed(conn, clearlogs = True):
    curs = conn.cursor()
    if clearlogs:
        curs.execute("SELECT jid FROM jobs WHERE status=4 OR status=6")
        for r in curs.fetchall():
            try: os.remove(get_job(r[0]).getpath())
            except: pass
    curs.execute("DELETE FROM jobs WHERE status=4 OR status=6")
    conn.commit()

######################
######################
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
    if type(rid) == type(""): rid = get_resource_id(curs,rid,rid)
    curs.execute("INSERT INTO resource_use(job_id, resource_id, quantity) VALUES (?,?,?)", (jid, rid, qty))

def get_job_resources(curs, jid):
    """Get list of resources requested by a job"""
    curs.execute("SELECT resource_id,name,quantity FROM resource_use NATURAL JOIN resources WHERE job_id = ?", (jid,))
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

        # check OK use
        for rs in job_rs:
            if rs[0] not in resuse: resuse[rs[0]] = check_resource_use(curs, rs[0])
            if resuse[rs[0]][0] < resuse[rs[0]][1] + rs[2]:
                res_ok = False
                break
        if not res_ok: continue

        for rs in job_rs: resuse[rs[0]][1] += rs[2]
        jout.append((jid, job_rs))

    return jout

###########
###########

def make_upload_shelljobs(curs, jname, q, acct, jcmds, res_use):
    """Upload set of shell script jobs with same requirements to DB; return IDs"""
    return [ShellJob(name=jname, q=q, acct=acct, script=jc, res_list = res_use).upload(curs) for jc in jcmds]

def rebundle(curs, tmax, nslots):
    """Bundle small jobs together into longer-time units"""

    curs.execute("SELECT quantity, job_id FROM jobs NATURAL JOIN resource_use \
        WHERE (status = 0 OR status = 8) AND jtype = 0 AND resource_id = ? AND quantity < ?", (find_resource_id(curs, "walltime"), tmax))
    js = list(curs.fetchall())
    if not js: return

    js.sort()
    j0 = get_job(curs, js[0][1])
    while True:
        B = BundleJob(name = "bundle")
        B.linear_order(js, nslots, tmax)
        if not B.runorder: break
        B.q = j0.q
        B.acct = j0.acct
        B.upload(curs)
        curs.executemany("UPDATE jobs SET status=7, associated=? WHERE job_id=?",[(B.jid, j) for j in B.runorder])
        print(B)
