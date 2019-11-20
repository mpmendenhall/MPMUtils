#! /bin/env python3
## @file JobManager_Local.py job submission on local system
# Michael P. Mendenhall, LLNL 2019

from JobManager_Interface import *

class LocalInterface(BatchQueueInterface):
    """Launch jobs locally without batch manager"""

    def __init__(self, conn):
        super().__init__(conn)

    def checkjob(self, qid):
        """Create job info dictionary for specified queue ID"""
        assert False

    def check_queued_status(self):
        """Get status on all queue items: [(QID, job state number, nnodes)]"""
        assert False
        curs.execute("SELECT job_id,queue_id FROM jobs WHERE status = 3 AND q_name = 'local'")
        for j in curs.fetchall():
            try: os.kill(j[1], 0)
            except:
                print("Job",j[0],j[1],"failed to report back!")
                curs.execute("UPDATE jobs SET status = 4, return_code = -999 WHERE job_id = ? AND status = 3", (j[0],))

    def _submit_job(self, jid):
        """Background-launch a job locally"""
        assert False # TODO
        curs.execute("SELECT jobscript,outlog FROM jobs WHERE job_id = ?", (jobid,))
        r = curs.fetchone()
        jcmd = r[0]
        if r[1]: jcmd += " > %s 2>&1"%r[1]
        jcmd += "; python3 %s --db %s --jid %i --setreturn $? --setstatus 4"%(__file__, dbfile, jobid)
        print(jcmd, flush=True)
        curs.execute("UPDATE jobs SET status = 1, t_submit = ? WHERE job_id = ?", (time.time(), jobid))
        pid = subprocess.Popen([jcmd,], shell=True, preexec_fn=(lambda : os.nice(15))).pid
        curs.execute("UPDATE jobs SET status = 3, queue_id = ? WHERE job_id = ? AND status != 4", (pid, jobid))
