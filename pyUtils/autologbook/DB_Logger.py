#!/usr/bin/python3
# should work in either python2 or 3

from sqlite3_RBU import *
import time

class DB_Logger:
    """Base class for writing data log"""

    def __init__(self, dbname):
        """Initialize with name of database to open"""
        self.conn = sqlite3.connect(dbname)
        self.curs = self.conn.cursor()

    def __del__(self):
        """Close DB connection on deletion"""
        if self.conn:
            self.conn.commit()
            self.conn.close()
            self.conn = None
    
    def get_inst_type(self, name):
        """Get instrument identifier by name"""
        self.curs.execute("SELECT rowid,name,descrip,dev_name,serial FROM instrument_types WHERE name = ?", (name,))
        r = self.curs.fetchall()
        return r[0] if len(r) == 1 else None
    
    def get_readout_type(self, name, inst_name = None):
        """Get identifier for readout by name and optional instrument name"""
        if inst_name is None:
            self.curs.execute("SELECT rowid FROM readout_types WHERE name = ?", (name,))
        else:
            self.curs.execute("SELECT readout_types.rowid FROM readout_types JOIN instrument_types ON instrument_id = instrument_types.rowid WHERE readout_types.name = ? AND instrument_types.name = ?", (name, inst_name))
        r = self.curs.fetchall()
        return r[0][0] if len(r) == 1 else None 
            
    def create_instrument(self, nm, descrip, devnm, sn, overwrite = False):
        """Assure instrument entry exists, creating/updating as needed"""
        self.curs.execute("INSERT OR " + ("REPLACE" if overwrite else "IGNORE") + " INTO instrument_types(name,descrip,dev_name,serial) VALUES (?,?,?,?)", (nm,descrip,devnm,sn))
        
    def create_readout(self, name, inst_name, descrip, units, overwrite = False):
        """Assure a readout exists, creating as necessary; return readout ID"""
        inst = self.get_inst_type(inst_name)
        if inst is None:
            return None
        self.curs.execute("INSERT OR " + ("REPLACE" if overwrite else "IGNORE") + " INTO readout_types(name,descrip,units,instrument_id) VALUES (?,?,?,?)", (name,descrip,units,inst[0]))
        self.curs.execute("SELECT rowid FROM readout_types WHERE name = ? AND instrument_id = ?", (name,inst[0]))
        r = self.curs.fetchall()
        return r[0][0] if len(r) == 1 else None
    
    def log_readout(self, tid, value, t = None):
        """Log reading, using current time for timestamp if not specified"""
        if t is None:
            t = time.time()
        self.curs.execute("INSERT INTO readings(type_id, time, value) VALUES (?,?,?)", (tid,value,t))
    
if __name__=="__main__":
    D = DB_Logger("test.db")
    D.create_instrument("funcgen", "test function generator", "ACME Foobar1000", "0001")
    r0 = D.create_readout("5min", "funcgen", "5-minute-period wave", None)
    r1 = D.create_readout("12h", "funcgen", "12-hour-period wave", None)
    D.conn.commit()
    
    from math import *
    while 1:
        t = time.time()
        v0 = sin(2*pi*t/300)
        v1 = sin(2*pi*t/(12*3600))
        D.log_readout(r0, v0, t)
        D.log_readout(r1, v1, t)
        D.conn.commit()
        print(t,v0,v1)
        time.sleep(5)
    