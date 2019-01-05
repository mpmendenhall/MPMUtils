#!/bin/env python3

# rm $ENDFDB
# sqlite3 $ENDFDB < ENDF6_DB_Schema.sql
# for f in ~/Data/ENDF-B-VII.1/*/*; do ./ENDF6_DB.py --load $f; done


import sqlite3
import os
from ENDF_Reader import *
from argparse import ArgumentParser
import pickle

class ENDFDB:
    """Connection to and manipulation of ENDF6 data DB"""

    def __init__(self,dbname = None):
        """Initialize with database filename to open"""
        if dbname is None: dbname = os.environ["ENDFDB"]
        self.conn = sqlite3.connect(dbname)
        self.conn.row_factory = sqlite3.Row # fast name-based access to columns
        self.curs = self.conn.cursor()
        self.readonly = False

    def find_section(self, MAT, MF, MT):
        """Return section_id for by MAT, MF, MT identifiers; None if absent."""
        self.curs.execute("SELECT section_id FROM ENDF_sections WHERE MAT=? AND MF=? AND MT=?", (MAT,MF,MT))
        res = self.curs.fetchall()
        return res[0][0] if res else None

    def find_sections(self, qdict):
        """Search sections table"""
        qvals = [("1 = ?", 1)] + [(k+" = ?", v) for k,v in qdict.items() if v is not None]
        cmd = "SELECT section_id FROM ENDF_sections WHERE " + " AND ".join([q[0] for q in qvals])
        self.curs.execute(cmd, [q[1] for q in qvals])
        return [r[0] for r in self.curs.fetchall()]

    def delete_section(self, sid):
        """Delete section information from database"""
        self.curs.execute("DELETE FROM ENDF_sections WHERE section_id = ?", (sid,))

    def upload_section(self, sec, txt, replace=True):
        """Upload file section to DB; return id number"""
        sid = self.find_section(sec.MAT, sec.MF, sec.MT)
        if sid is not None:
            if replace: self.delete_section(sid)
            else: return sid

        try:
            s = load_ENDF_Section(iter(txt.split('\n')))
            assert s is not None
            self.curs.execute("INSERT INTO ENDF_sections(MAT,MF,MT,A,Z,pcl) VALUES(?,?,?,?,?,?)", (sec.MAT, sec.MF, sec.MT, sec.A, sec.Z, pickle.dumps(s)))
        except:
            print("\n**** Unable to pre-parse ****")
            print(sec)
            self.curs.execute("INSERT INTO ENDF_sections(MAT,MF,MT,A,Z,lines) VALUES(?,?,?,?,?,?)", (sec.MAT, sec.MF, sec.MT, sec.A, sec.Z, txt))

        return self.curs.lastrowid

    def get_section(self, sid):
        """Return section from DB"""
        self.curs.execute("SELECT lines, pcl FROM ENDF_sections WHERE section_id = ?", (sid,))
        res = self.curs.fetchall()
        if not res: return None
        if res[0][1]: return pickle.loads(res[0][1])

        ls = res[0][0].split("\n")
        try: s = load_ENDF_Section(iter(ls))
        except:
            h = ENDF_Record(ls[0])
            print("Unhandled type",h)
            return None
        if s is not None and not self.readonly:
            self.curs.execute("UPDATE ENDF_sections SET lines=?, pcl=? WHERE section_id=?", (None, pickle.dumps(s), sid))
            self.conn.commit()
        return s

if __name__=="__main__":

    parser = ArgumentParser()
    parser.add_argument("--db",     help="location of DB file")
    parser.add_argument("--load",   help="import input file to DB")
    parser.add_argument("--A",      type=int,   help="filter by A")
    parser.add_argument("--Z",      type=int,   help="filter by Z")
    parser.add_argument("--MF",     type=int,   help="filter by file number")
    parser.add_argument("--MT",     type=int,   help="filter by file section")
    parser.add_argument("--display", action="store_true", help="Display located sections")
    parser.add_argument("--count",   action="store_true", help="Count entries matching query")

    options = parser.parse_args()
    EDB = ENDFDB(options.db)

    if options.load:

        print("Loading", options.load)
        f = open(options.load,"r")

        # tape header line
        h0 = ENDF_Record(next(f))
        assert h0.MF == h0.MT == 0 and h0.MAT == 1

        nloaded = 0
        while f:
            ls = pop_section_lines(f)
            h = ENDF_HEAD_Record(ls[0])
            if h.rectp == "TEND": break
            if not h.endlvl:
                EDB.upload_section(h, ''.join(ls))
                nloaded += 1

        EDB.conn.commit()
        print("\tLoaded", nloaded, "entries.")

    sids = []
    if options.display or options.count:
        sids = EDB.find_sections({"A": options.A, "Z": options.Z, "MF": options.MF, "MT": options.MT})
        print("Found", len(sids), "matching records.")
    if options.display:
        for s in sids:
            print("\n--------------------------------------")
            print(EDB.get_section(s))
