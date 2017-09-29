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

def get_process_id(curs, name, descrip):
    """Find or create new named process; return ID"""
    curs.execute("SELECT process_id FROM process WHERE name = ?", (name,))
    res = curs.fetchall()
    if len(res) == 1 : return res[0][0]
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
    curs.execute("UPDATE status SET state = ?, stattime = strftime('%s', 'now') WHERE entity_id = ? AND process_id = ?", (s,eid,pid))

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

    def find_ready(self):
        """Find entities ready for this step"""
        cquery = "SELECT entity_id FROM "
        if self.prereqs: cquery += " INTERSECT ".join(["(SELECT entity_id FROM status WHERE process_id = ? AND state = 2)" for pid in self.prereqs])
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
                isdone = self.check_if_already_done(eid)
                success = res[2] == 0 and isdone
                if not success: print("** WARNING ** process '%s' on '%s' failed ( return code %i, done check"%(self.name, get_entity_info(self.curs,eid)[0], res[2]), isdone, ")")
                self.set_status(eid, 2 if success else 3)
                self.curs.execute("UPDATE status SET  calctime = ? WHERE entity_id = ? AND process_id = ?", (res[1], eid, self.pid))

    def check_if_already_done(self, eid):
        """Placeholder; check whether processing has already been done"""
        return True

    def start_process(self, rdbcurs, eid):
        """Start job for process (handed off to JobManager)"""
        einfo = get_entity_info(self.curs, eid)
        jcmd = self.job_command(eid, einfo[0])
        print(self.name,"starting process for",einfo[0])
        print(jcmd)
        if self.dryrun: return
        self.set_status(eid, 0)
        jid = make_job_script(rdbcurs, self.name, jcmd, self.res_use)
        assert jid is not None
        self.curs.execute("UPDATE status SET job_id = ? WHERE entity_id = ? AND process_id = ?", (jid, eid, self.pid))
        self.set_status(eid, 1)

    def job_command(self, eid, ename):
        """Placeholder: job command to run to process entity"""
        return """echo '%s says Hello World for %s'"""%(self.name, ename)

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
