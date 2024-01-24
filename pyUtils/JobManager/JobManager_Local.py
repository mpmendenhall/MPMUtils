#! /bin/env python3
## @file JobManager_Local.py job submission on local system
# Michael P. Mendenhall, LLNL 2019

from JobManager_Interface import *
import subprocess

class LocalInterface(BatchQueueInterface):
    """Launch jobs locally without batch manager"""

    def __init__(self, conn):
        super().__init__(conn)

    def nbundle(self):
        """Recommended bundling parallelism"""
        return cpu_count()

    def check_queued_status(self):
        """Get status on all queue items: [(QID, job state number)]"""
        return []

        jstatus = []
        self.curs.execute("SELECT queue_id FROM jobs WHERE 1 <= status AND status <= 3")
        for j in self.curs.fetchall():
            try: os.kill(j[0], 0)
            except: continue
            print("Job", j[0], "still running")
            jstatus.append((j[0], 3))
        return jstatus

    def _submit_job(self, J):
        """Background-launch a job locally"""
        J.make_wrapper()
        d = J.getpath()
        out = open(d+"/log.txt", "w")
        return subprocess.Popen(["bash", d + "/wrapper.sh"], stdout = out, stderr = out).pid
