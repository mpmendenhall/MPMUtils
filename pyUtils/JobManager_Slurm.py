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

    def nbundle(self):
        """Recommended bundling parallelism"""
        return 36 if "boraxo" in os.uname()[1] else 1

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

    def _submit_job(self, J):
        """Submit job via msub"""

        p = J.getpath()
        mcmds =["-j oe", "-V",
                "-l ttc=%i"%J.res_list["cores"],
                "-l walltime=%i"%J.res_list["walltime"],
                "-o " + p + "/log.txt"]
        if J.name: mcmds.append("-N %s_%i"%(J.name, J.jid))
        if J.q: mcmds.append("-q "+ J.q)
        if J.acct: mcmds.append("-A " + J.acct)

        mscript = '\n'.join(["#!/bin/bash",] + ["#MSUB " + m for m in mcmds]) + '\n\n'

        mscript += J.prerun_script() + "\n"
        mscript += " ".join(J.get_run_command()) + '\n\n'
        mscript += J.postrun_script() + '\n'
        open(p+"/msub.txt", "w").write(mscript)

        msub = subprocess.Popen(["msub"], stdout = subprocess.PIPE, stdin = subprocess.PIPE, stderr = subprocess.PIPE)
        msub.stdin.write(mscript.encode())
        ou,er = msub.communicate()

        o = ou.decode().strip()
        print("Job '%s' submitted as '%s'"%(J.jid,o))

        try:
            qid = int(o.split()[-1].split(".")[0])
            return qid
        except:
            print("*** msub error ***")
            print(er.decode())
            return None

    def kill_jobs(self, usr = None, account = None):
        """Force kill all running jobs"""
        cmd = ["scancel", "-u", usr if usr is not None else os.environ["USER"]]
        if account: cmd += ["-A", account]
        subprocess.call(cmd)
