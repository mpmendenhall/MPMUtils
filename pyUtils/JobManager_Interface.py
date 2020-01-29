#! /bin/env python3
## @file JobManager_Interface.py Base interface to submission systems
# Michael P. Mendenhall, LLNL 2019

from JobManager_DB import *

class BatchQueueInterface:
    """Base interface for batch queue"""

    def __init__(self, conn):
        self.conn = conn
        self.curs = conn.cursor()

    def update_qstatus(self):
        """Update database job status"""
        jstatus = self.check_queued_status()

        self.curs.executemany("UPDATE jobs SET status=? WHERE queue_id=?",[(j[1],j[0]) for j in jstatus])

        jknown = frozenset([j[0] for j in jstatus if 1 <= j[1] <= 3])
        self.curs.execute("SELECT job_id, queue_id FROM jobs WHERE 1 <= status AND (status <= 3 OR status == 5) AND (q_name is NULL OR q_name != 'local')")
        missingjobs = [(j[0], self.checkjob(j[1])) for j in self.curs.fetchall() if j[1] not in jknown]
        missingjobs = [(j[1]["status"], j[1].get("ret_code",None), j[1].get("walltime",None), j[0]) for j in missingjobs]
        if missingjobs: self.curs.executemany("UPDATE jobs SET status=?,return_code=?,use_walltime=? WHERE job_id=?", missingjobs)
        for j in missingjobs:
            if j[0] in (4, 6): post_completion(self.curs, j[-1])

        self.conn.commit()

    def submit_job(self, jid):
        """Submit specified job through batch system interface"""
        J = get_job_info(self.curs, jid)
        if J is None: return

        qid = self._submit_job(J)
        if qid is not None:
            self.curs.execute("UPDATE jobs SET queue_id=?, status=1, t_submit = ? WHERE job_id = ?", (qid, time.time(), jid))
            self.conn.commit()
        else: print("FAILED to submit job!")

    def update_and_launch(self, trickle = None):
        """Update status; launch new jobs as available"""
        self.update_qstatus()

        jnext = get_possible_submissions(self.curs)
        if len(jnext): print("Submitting %i new jobs."%len(jnext))
        for j in jnext:
            self.submit_job(j[0])
            if trickle and j != jnext[-1]: time.sleep(trickle)

        summarize_DB_runstatus(self.curs)

    def cycle_launch(self, trickle=None, twait=15):
        """Continue launching jobs until all submitted"""
        n = 1
        while n:
            self.curs.execute("SELECT COUNT(*) FROM jobs WHERE status IN (0,1,2,3)")
            nleft = self.curs.fetchone()[0]
            if not nleft: break
            if twait and n > 1: time.sleep(twait)
            n += 1
            print(nleft, "jobs uncompleted.", flush=True)
            self.update_and_launch(trickle)

    def cancel_queued_jobs(self):
        """Cancel all queued but not running jobs"""
        self.update_qstatus()
        self.curs.execute("SELECT queue_id FROM jobs WHERE queue_id > 0 AND status <= 2")
        assert False #for j in curs.fetchall(): os.system("mjobctl -c %i"%j)
        self.curs.execute("UPDATE jobs SET status = 6 WHERE status = 0")
        self.conn.commit()
        summarize_DB_runstatus(self.curs)

###########################
############## Subclass me:

    def checkjob(self, qid):
        """Create job info dictionary for specified queue ID"""
        assert False

    def check_queued_status(self):
        """Get status on all queue items: [(QID, job state number, nnodes)]"""
        assert False

    def _submit_job(self, J):
        """Submit specified job through batch system interface; return queue system ID or None if failed"""
        return None

    def kill_jobs(self):
        """Force kill all jobs"""
        assert False
