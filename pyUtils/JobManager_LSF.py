#! /bin/env python3
## @file JobManager_LSF.py job submission interface to LSF/bsub/jsrun job control
# Michael P. Mendenhall, LLNL 2019

from JobManager_Interface import *
import subprocess

class LSFInterface(BatchQueueInterface):

    jobstates = {"PEND":2, "PSUSP":2, "SSUSP":2, "USUSP":2, "RUN":3, "DONE":4, "UNKWN":5, "EXIT":6}

    def __init__(self, conn):
        super().__init__(conn)

    def check_queued_status(self):
        """Get status on all queue items: [(QID, job state number)]"""
        cmd = 'bjobs'
        qdat = subprocess.getoutput(cmd)
        print(cmd)
        print(qdat)
        jstatus = []
        for l in qdat.split("\n"):
            if l[0] not in "0123456789": continue
            l = l.split()
            jstatus.append((int(l[0]), self.jobstates.get(l[2],5)))
        return jstatus

    def checkjob(self, qid):
        """Create job info dictionary for specified queue ID"""
        jinfo = subprocess.check_output(["bjobs","-l",str(qid)]).decode()
        print(jinfo)
        ldat = {"status": 5}
        for l in jinfo.split('\n'):
            if "Done successfully." in l:
                ldat["status"] = 4
                ldat["ret_code"] = 0
            if "Exited with exit code" in l:
                ldat["status"] = 6
                ldat["ret_code"] = int(l.split("exit code ")[1].split(".")[0])
        return ldat

    def _submit_job(self, J):
        """Submit job via bsub"""

        mcmds = ["-nnodes 1",
                 "-ln_slots %i"%J.ncores,
                 "-W %i"%(1.+J.wtreq/60.)]
        if J.acct: mcmds.append("-G "+J.acct)
        if J.logfile: mcmds.append("-o "+J.logfile)
        if J.name: mcmds.append("-J "+J.name)
        if J.q: mcmds.append("-q "+J.q)

        cmd = '\n'.join(["#BSUB "+m for m in mcmds]+['\n', J.script])
        bsub = subprocess.Popen(["bsub"], stdout = subprocess.PIPE, stdin = subprocess.PIPE, stderr = subprocess.PIPE)
        bsub.stdin.write(cmd.encode())
        ou,er = bsub.communicate()
        o = ou.decode().strip()
        e = er.decode().strip()
        if e: print("\n", e, "\n")
        print("Job %s -> '%s'"%(J.jid, o))

        try: # e.g. "Job <689944> is submitted to queue <pbatch0>."
            qid = int(o.split()[1][1:-1])
            return qid
        except: return None

    def kill_jobs(self, usr = None, account = None):
        """Force kill all running jobs"""
        assert False
