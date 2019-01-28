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
        self.curs.execute("PRAGMA foreign_keys=ON")
        self.readonly = False
        self.letFail = False
        self.cache = {} # set to None to disable

    def find_section(self, MAT, MF, MT):
        """Return section_id(s) for by MAT, MF, MT identifiers"""
        self.curs.execute("SELECT section_id FROM ENDF_sections WHERE MAT=? AND MF=? AND MT=?", (MAT,MF,MT))
        return [r[0] for r in self.curs.fetchall()]

    def find_sections(self, qdict):
        """Search sections table"""
        qvals = [("1 = ?", 1)] + [(k+" = ?", v) for k,v in qdict.items() if v is not None]
        cmd = "SELECT section_id FROM ENDF_sections WHERE " + " AND ".join([q[0] for q in qvals]) + " ORDER BY MAT"
        self.curs.execute(cmd, [q[1] for q in qvals])
        return [r[0] for r in self.curs.fetchall()]

    def find_F8MT457(self, A, Z, LISO):
        self.curs.execute("SELECT section_id FROM MF8_MT457_directory WHERE A=? AND Z=? AND LISO=?", (A, Z, LISO))
        r = self.curs.fetchone()
        return r[0] if r else None

    def delete_section(self, sid):
        """Delete section information from database"""
        self.curs.execute("DELETE FROM ENDF_sections WHERE section_id = ?", (sid,))

    def upload_section(self, sec, txt, replace = None):
        """Upload file section to DB; return id number"""
        if replace is None: replace = sec.MF != 1 or sec.MT != 451
        if replace:
            sids = self.find_section(sec.MAT, sec.MF, sec.MT)
            for sid in sids: self.delete_section(sid)

        if self.letFail:
            s = load_ENDF_Section(iter(txt.split('\n')))
            assert s is not None
            self.curs.execute("INSERT INTO ENDF_sections(MAT,MF,MT,A,Z,pcl) VALUES(?,?,?,?,?,?)", (sec.MAT, sec.MF, sec.MT, sec.A, sec.Z, pickle.dumps(s)))
        else:
            try:
                if not self.letFail: s = load_ENDF_Section(iter(txt.split('\n')))
                assert s is not None
                self.curs.execute("INSERT INTO ENDF_sections(MAT,MF,MT,A,Z,pcl) VALUES(?,?,?,?,?,?)", (sec.MAT, sec.MF, sec.MT, sec.A, sec.Z, pickle.dumps(s)))
            except:
                print("\n**** Unable to pre-parse ****")
                print(sec)
                self.curs.execute("INSERT INTO ENDF_sections(MAT,MF,MT,A,Z,lines) VALUES(?,?,?,?,?,?)", (sec.MAT, sec.MF, sec.MT, sec.A, sec.Z, txt))

        sid = self.curs.lastrowid
        if sec.MF == 8 and sec.MT == 457:
            self.curs.execute("INSERT INTO MF8_MT457_directory(section_id,A,Z,LIS,LISO) VALUES(?,?,?,?,?)", (sid, s.A, s.Z, s.LIS, s.LISO))

        return sid

    def get_section(self, sid):
        """Return section from DB"""
        if self.cache is not None and sid in self.cache: return self.cache[sid]

        self.curs.execute("SELECT lines, pcl FROM ENDF_sections WHERE section_id = ?", (sid,))
        res = self.curs.fetchall()
        if not res:
            print("Section id",sid,"not in database!")
            return None
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
        if self.cache is not None: self.cache[sid] = s
        return s

