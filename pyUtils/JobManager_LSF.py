#! /bin/env python3
## @file JobManager_LSF.py job submission interface to LSF/bsub/jsrun job control
# Michael P. Mendenhall, LLNL 2020

from JobManager_Interface import *
import subprocess

class LSFInterface(BatchQueueInterface):

    jobstates = {"PEND":2, "PSUSP":2, "SSUSP":2, "USUSP":2, "RUN":3, "DONE":4, "UNKWN":5, "EXIT":6}

    def __init__(self, conn):
        super().__init__(conn)

    def nbundle(self):
        """Recommended bundling parallelism"""
        return 40

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

    def _submit_job(self, J):
        """Submit job via bsub"""

        p = J.getpath()
        nn = (1 + (int(J.res_list["cores"]) - 1)//40)
        assert nn == 1
        mcmds = ["-nnodes %i"%nn, "-env all",
                 "-W %i"%(5. + J.res_list["walltime"]/60.),
                 "-o " + p + "/log.txt"]
        if J.name: mcmds.append("-J " + J.name)
        if J.acct: mcmds.append("-G "+J.acct)
        if J.q: mcmds.append("-q " + J.q)

        bscript = '\n'.join(["#!/bin/bash",] + ["#BSUB " + m for m in mcmds]) + '\n\n'

        bscript += J.prerun_script() + "\n"
        bscript += " ".join(J.get_run_command()) + '\n\n'
        bscript += J.postrun_script() + '\n'
        open(p+"/bsub.txt", "w").write(bscript)

        bsub = subprocess.Popen(["bsub"], stdout = subprocess.PIPE, stdin = subprocess.PIPE, stderr = subprocess.PIPE)
        bsub.stdin.write(bscript.encode())

        ou,er = bsub.communicate()
        o = ou.decode().strip()
        e = er.decode().strip()
        if e: print("\n", e, "\n")
        print("Job %s -> '%s'"%(J.jid, o))

        try: # e.g. "Job <689944> is submitted to queue <pbatch0>."
            qid = int(o.split()[1][1:-1])
            return qid
        except: return None
