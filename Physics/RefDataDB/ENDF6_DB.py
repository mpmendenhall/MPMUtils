import sqlite3
import os
from ENDF_Reader import *
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
        self.letFail = False

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

        if self.letFail: s = load_ENDF_Section(iter(txt.split('\n')))
        try:
            if not self.letFail: s = load_ENDF_Section(iter(txt.split('\n')))
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
        if self.letFail: s = load_ENDF_Section(iter(ls))
        else:
            try: s = load_ENDF_Section(iter(ls))
            except:
                h = ENDF_Record(ls[0])
                print("Unhandled type",h)
                return None
        if s is not None and not self.readonly:
            self.curs.execute("UPDATE ENDF_sections SET lines=?, pcl=? WHERE section_id=?", (None, pickle.dumps(s), sid))
            self.conn.commit()
        return s

