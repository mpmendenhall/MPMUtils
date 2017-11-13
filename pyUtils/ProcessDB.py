#! /bin/env python3
## @file ProcessDB.py Processing steps database interface

import sqlite3
import os
from JobManager import *

pdb_state_names = {0: "setup", 1: "in progress", 2: "done", 3: "failed"}

def connect_ProcessDB(fname):
    """Get cursor and connection for ProcessDB"""
    conn = sqlite3.connect(fname)
    curs = conn.cursor()
    curs.execute("PRAGMA foreign_keys = ON")
    return curs,conn

def find_entity_id(curs, name):
    """Find named entity, if it exists"""
    curs.execute("SELECT entity_id FROM entity WHERE name = ?", (name,))
    res = curs.fetchall()
    return res[0][0] if len(res) == 1 else None

def create_entity(curs, name, descrip):
    """Create named entity; return id"""
    curs.execute("INSERT INTO entity(name,descrip) VALUES (?,?)", (name, descrip))
    return curs.lastrowid

def get_entity_id(curs, name, descrip):
    """Find or create new named entity; return ID"""
    eid = find_entity_id(curs, name)
    if eid is not None: return eid
    return create_entity(curs, name, descrip)

def find_process_id(curs, name):
    """Find process ID by name"""
    curs.execute("SELECT process_id FROM process WHERE name = ?", (name,))
    res = curs.fetchall()
    return res[0][0] if len(res) == 1 else None

def get_process_id(curs, name, descrip):
    """Find or create new named process; return ID"""
    pid = find_process_id(curs,name)
    if pid is not None: return pid
    curs.execute("INSERT INTO process(name,descrip) VALUES (?,?)", (name, descrip))
    return curs.lastrowid

def get_entity_info(curs, eid):
    """Get name/description for entity"""
    curs.execute("SELECT name,descrip FROM entity WHERE entity_id = ?", (eid,))
    return curs.fetchall()[0]

def get_process_info(curs, pid):
    """Get name/description for process"""
    curs.execute("SELECT name,descrip FROM process WHERE process_id = ?", (pid,))
    return curs.fetchall()[0]

def set_status(curs, eid, pid, s):
    """Set status of process for entity"""
    curs.execute("INSERT OR IGNORE INTO status(entity_id, process_id) VALUES  (?,?)", (eid,pid))
    curs.execute("UPDATE status SET state = ?, stattime = ? WHERE entity_id = ? AND process_id = ?", (s,time.time(),eid,pid))

def clear_process_status(curs, pid):
    """Delete status info for processing, to cause re-processing"""
    curs.execute("DELETE FROM status WHERE process_id = ?", (pid,))

def display_pdb_summary(curs):
    """Summary stats for process DB"""
    print("\n--- Process DB Status ---")

    curs.execute("SELECT process_id FROM process")
    pids = [r[0] for r in curs.fetchall()]

    for pid in pids:
        print("* %s: %s"%get_process_info(curs, pid))
        for i in range(4):
            curs.execute("SELECT COUNT(*) FROM status WHERE process_id = ? AND state = ?", (pid, i))
            nstate = curs.fetchone()[0]
            if nstate: print("\t%i\toperations"%nstate,pdb_state_names[i])

    print("--- total ----")
    curs.execute("SELECT COUNT(*) FROM entity")
    print(curs.fetchone()[0],"entities")
    for i in range(4):
        curs.execute("SELECT COUNT(*) FROM status WHERE state = ?", (i,))
        print("%i\toperations"%curs.fetchone()[0],pdb_state_names[i])
    print()

class ProcessStep:
    """Base class for a step with pre-requisites"""
    def __init__(self, curs, name, descrip):
        self.curs = curs
        self.name = name
        self.pid = get_process_id(curs,name,descrip)
        self.info = get_process_info(curs,self.pid)
        self.prereqs = [] # required completed process IDs to apply this step
        self.res_use = [("cores",1),]
        self.dryrun = False
        self.input_tmax = datetime.datetime(2100, 1, 1, 0, 0, 0).timestamp() # upper limit on input prereq time

    def find_ready(self):
        """Find entities ready for this step"""
        cquery = "SELECT entity_id FROM "
        if self.prereqs: cquery += " INTERSECT ".join(["(SELECT entity_id FROM status WHERE process_id = ? AND state = 2 AND stattime < %i)"%self.input_tmax for pid in self.prereqs])
        else: cquery += "entity"
        cquery += " WHERE entity_id NOT IN (SELECT entity_id FROM status WHERE process_id = ?)"
        # print(cquery)
        self.curs.execute(cquery, self.prereqs+[self.pid,])
        return [r[0] for r in self.curs.fetchall()]

    def set_status(self, eid, s):
        """Set entity status for this process"""
        set_status(self.curs, eid, self.pid, s)

    def check_progress(self, rdbcurs):
        """Check in-progress operations at this step for completion/failure"""
        self.curs.execute("SELECT entity_id, job_id FROM status WHERE state = 1 AND process_id = ?", (self.pid,))
        for eid,jid in self.curs.fetchall():
            assert jid is not None
            rdbcurs.execute("SELECT status, use_walltime, return_code FROM jobs WHERE job_id = ?", (jid,))
            res = rdbcurs.fetchall()
            if len(res) != 1:
                print("** WARNING ** process '%s' on '%s' submitted but not found in JobsDB [%i]!"%(self.name, get_entity_info(self.curs,eid)[0],jid))
                self.set_status(eid, 3)
                continue
            res = res[0]
            if res[0] == 4: # job is finished
                ename = get_entity_info(self.curs, eid)[0]
                isdone = self.check_if_already_done(eid, ename)
                success = res[2] == 0 and isdone
                if not success: print("** WARNING ** process '%s' on '%s' failed ( return code %i, done check"%(self.name, get_entity_info(self.curs,eid)[0], res[2]), isdone, ")")
                self.set_status(eid, 2 if success else 3)
                self.curs.execute("UPDATE status SET  calctime = ? WHERE entity_id = ? AND process_id = ?", (res[1], eid, self.pid))

    def check_if_already_done(self, eid, ename):
        """Placeholder; check whether processing has already been done"""
        return True

    def start_process(self, rdbcurs, eid):
        """Start job for process (handed off to JobManager)"""
        ename = get_entity_info(self.curs, eid)[0]
        jcmd = self.job_command(eid, ename)
        print(self.name,"starting process for",ename)
        print(jcmd)
        if self.dryrun: return
        self.set_status(eid, 0)
        estrt = self.estimate_runtime(eid,ename)
        if estrt is None: estrt = 1800
        jid = make_job_script(rdbcurs, self.name, jcmd, self.res_use + [("walltime",estrt)])
        assert jid is not None
        self.curs.execute("UPDATE status SET job_id = ? WHERE entity_id = ? AND process_id = ?", (jid, eid, self.pid))
        self.set_status(eid, 1)

    def job_command(self, eid, ename):
        """Placeholder: job command to run to process entity"""
        return """echo '%s says Hello World for %s'"""%(self.name, ename)

    def input_file(self, eid, ename):
        """Placeholder: return input file name"""
        return None

    def output_file(self, eid, ename):
        """Placeholder: return output filename"""
        return None

    def estimate_runtime(self, eid, ename):
        """Placeholder: return estimate for job run time [seconds]"""
        return None

    class JobProfile:
        """Collection of summary data for a completed job"""
        def __init__(self, PS, eid, ename):
            self.ename = ename
            self.infile = PS.input_file(eid,ename)
            self.outfile = PS.output_file(eid,ename)
            self.inflsize = os.path.getsize(self.infile) if self.infile and os.path.exists(self.infile) else None
            self.outflsize = os.path.getsize(self.outfile) if self.outfile and os.path.exists(self.outfile) else None

    def profile_jobs(self):
        """Return list of completed job profiles"""
        self.curs.execute("SELECT entity_id,stattime,calctime FROM status WHERE process_id = ? AND state = 2", (self.pid,))
        jps = []
        for eid,st,ct in self.curs.fetchall():
            ename = get_entity_info(self.curs, eid)[0]
            jps.append(self.JobProfile(self,eid,ename))
            jps[-1].rundate = st
            jps[-1].calctime = ct
        return jps

##########################
if __name__ == "__main__":

    os.system("rm testProcessDB.sql")
    os.system("sqlite3 testProcessDB.sql < ProcessDB_Schema.sql")
    curs,conn = connect_ProcessDB("testProcessDB.sql")

    # status to indicate an entity exists; used as prerequisite for future actions
    pEx = get_process_id(curs, "exists", "the thing exists")

    # process to eat anything that exists
    pEat = ProcessStep(curs, "eat", "eat the thing")
    pEat.prereqs.append(pEx) # thing must exist to be eaten

    # create two entities with "existence" status
    eApple = get_entity_id(curs, "apple", "an apple")
    set_status(curs, eApple, pEx , 2) # the apple exists
    ePear = get_entity_id(curs, "pear", "a pear")
    set_status(curs, ePear, pEx , 2) # the pear exists

    print("Ready to be eaten:")
    for eid in pEat.find_ready(): print(get_entity_info(curs,eid))

    set_status(curs, eApple, pEat.pid, 2) # the apple has been eaten

    print("Ready to be eaten:")
    for eid in pEat.find_ready(): print(get_entity_info(curs,eid))

    conn.commit()
