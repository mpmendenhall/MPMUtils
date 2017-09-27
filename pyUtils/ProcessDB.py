#! /bin/env python3
## @file ProcessDB.py Processing steps database interface

import sqlite3
import os

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
    curs.execute("INSERT OR REPLACE INTO status(entity_id, process_id, state, time) values (?,?,?,strftime('%s', 'now'))", (eid,pid,s))


class ProcessStep:
    """Base class for a step with pre-requisites"""
    def __init__(self, curs, name, descrip):
        self.curs = curs
        self.pid = get_process_id(curs,name,descrip)
        self.info = get_process_info(curs,self.pid)
        self.prereqs = [] # required completed process IDs to apply this step

    def find_ready(self):
        """Find entities ready for this step"""
        cquery = "SELECT entity_id FROM "
        if self.prereqs: cquery += " INTERSECT ".join(["(SELECT entity_id FROM status WHERE process_id = ? AND state = 2)" for pid in self.prereqs])
        else: cquery += "entity"
        cquery += " WHERE entity_id NOT IN (SELECT entity_id FROM status WHERE process_id = ?)"
        # print(cquery)
        self.curs.execute(cquery, self.prereqs+[self.pid,])
        return [r[0] for r in self.curs.fetchall()]

    def check_if_already_done(self, eid):
        """Placeholder; check whether processing has already been done"""
        return False

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
