#! /bin/env python3
## @file JobManager_Slurm.py job submission interface to "slurm"/msub job control
# Michael P. Mendenhall, LLNL 2019

from JobManager_Interface import *
import subprocess
import datetime

class SlurmInterface(BatchQueueInterface):
    jobstates = {"Idle":2, "ReqNodeNotA":2, "Starting":3, "Running":3, "Completed":4, "Unknown":5, "Removed":6, "Bundled":7}

    def __init__(self, conn):
        super().__init__(conn)

    def check_queued_status(self):
        """Get status on all queue items: [(QID, job state number)]"""
        cmd = 'showq -u $USER'
        qdat = subprocess.getoutput(cmd)
        print(cmd)
        print(qdat)
        jstatus = []
        for l in qdat.split("\n"):
            l = l.split()
            if len(l) != 9: continue
            jstatus.append((int(l[0]), self.jobstates.get(l[2],5)))
        return jstatus

    def checkjob(self, qid):
        """Create job info dictionary for specified queue ID"""
        print("Checking job status for", qid)

        jdat = {}
        for j in subprocess.getoutput("checkjob %i"%qid).split("\n"):
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
        jdat["status"] = self.jobstates.get(jdat.get("State","unknown"),5)
        print(jdat)
        return jdat

    def _submit_job(self, J, mcmds=["-j oe", "-V"]):
        """Submit job via msub"""

        cmd = ["msub", "-l", "nodes=%i"%J.ncores, "-l", "walltime=%i"%J.wtreq]
        if J.name: cmd += ["-N", "%s_%i"%(J.name, J.jid)]
        if J.logfile: cmd += ["-o", J.logfile]
        if J.q: cmd += ["-q", J.q]
        if J.acct: cmd += ["-A", J.acct]

        msub = subprocess.Popen(cmd, stdout = subprocess.PIPE, stdin = subprocess.PIPE, stderr = subprocess.PIPE)
        msub.stdin.write('\n'.join(["#MSUB "+m for m in mcmds]+[J.script,]).encode())
        ou,er = msub.communicate()

        o = ou.decode().strip()
        print("Job '%s' submitted as '%s'"%(J.jid,o))

        try:
            qid = int(o.split()[-1].split(".")[0])
            return qid
        except: return None

    def kill_jobs(self, usr = None, account = None):
        """Force kill all running jobs"""
        cmd = ["scancel", "-u", usr if usr is not None else os.environ["USER"]]
        if account: cmd += ["-A", account]
        subprocess.call(cmd)
