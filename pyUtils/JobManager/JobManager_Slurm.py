#! /bin/env python3
## @file JobManager_Slurm.py job submission interface to slurm/sbatch job control
# Michael P. Mendenhall, LLNL 2019

from JobManager_Interface import *
import subprocess
import datetime

class SlurmInterface(BatchQueueInterface):
    jobstates = {"PENDING":2, "RUNNING":3, "COMPLETED":4, "Unknown":5, "CANCELLED":6, "TIMEOUT":8, "NODE_FAIL":9, "Bundled":7}

    def __init__(self, conn):
        super().__init__(conn)

    def nbundle(self):
        """Recommended bundling parallelism"""
        return 36 if "boraxo" in os.uname()[1] else 1

    def check_queued_status(self):
        """Get status on all queue items: [(QID, job state number)]"""
        cmd = 'sacct -u $USER --format=JobID,state'
        qdat = subprocess.getoutput(cmd)
        print(cmd)
        print(qdat)
        jstatus = []
        for l in qdat.split("\n"):
            l = l.split()
            if len(l) != 2 or not l[0].isdigit(): continue
            jstatus.append((int(l[0]), self.jobstates.get(l[1].strip("+"),5)))
        return jstatus

    def _submit_job(self, J):
        """Submit job via msub"""

        p = J.getpath()
        mcmds =["--export=ALL",
                "-n %i"%J.res_list["cores"],
                "-t %i"%(J.res_list["walltime"]/60),
                "-o " + p + "/log.txt"]
        if J.name: mcmds.append("-J %s_%i"%(J.name, J.jid))
        if J.q: mcmds.append("-p " + J.q)
        if J.acct: mcmds.append("-A " + J.acct)

        mscript = '\n'.join(["#!/bin/bash",] + ["#SBATCH " + m for m in mcmds]) + '\n\n'

        mscript += J.prerun_script() + "\n"
        mscript += " ".join(J.get_run_command()) + '\n\n'
        mscript += J.postrun_script() + '\n'
        open(p+"/sbatch.txt", "w").write(mscript)

        sbatch = subprocess.Popen(["sbatch"], stdout = subprocess.PIPE, stdin = subprocess.PIPE, stderr = subprocess.PIPE)
        sbatch.stdin.write(mscript.encode())
        ou,er = sbatch.communicate()

        o = ou.decode().strip()
        print("Job '%s' submitted as '%s'"%(J.jid,o))

        try:
            qid = int(o.split()[-1].split(".")[0])
            return qid
        except:
            print("*** sbatch error ***")
            print(er.decode())
            return None

    def kill_jobs(self, usr = None, account = None):
        """Force kill all running jobs"""
        cmd = ["scancel", "-u", usr if usr is not None else os.environ["USER"]]
        if account: cmd += ["-A", account]
        subprocess.call(cmd)
