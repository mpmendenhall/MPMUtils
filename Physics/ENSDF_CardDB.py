#!/usr/bin/python3
"""@package ENSDF_CardDB
Database interface for storing and retrieving ENSDF (uninterpreted) card information"""

import sqlite3
import os

def upload_card(curs, lines):
    """Store one card, split into lines, to DB"""
    if not lines: return
    #print(lines[0])
    
    l = lines[0]
    mass = int(l[:3])
    elem = l[3:5].strip()
    DSID = l[9:39].strip()
    DSREF = l[39:65].strip()
    PUB = l[65:74].strip()
    EDATE = int(l[74:80]) if l[74:80].strip() else 0
    
    curs.execute("INSERT INTO ENSDF_cards VALUES (?,?,?,?,?,?)", (mass,elem,DSID,DSREF,PUB,EDATE))
    cid = curs.lastrowid
    
    ldata = []
    for l in lines:
        if not l.strip(): continue
        try:
            ldata.append((cid, int(l[:3]), l[3:5].strip(), l[5:8], l[8:80]))
        except:
            print("Failed line: '%s'"%l)
            raise
    curs.executemany("INSERT INTO ENSDF_lines VALUES (?,?,?,?,?)", ldata)
  
def upload_cards(curs, txt):
    """Upload many cards split from file"""
    lns = []
    for l in txt.split("\n"): 
        if not l.strip():
            upload_card(curs,lns)
            lns = []
        else:
            lns.append(l)
    upload_card(curs,lns)
    
def upload_dataset(curs,basedir):
    """Upload entire dataset from directory"""
    curs.execute("DELETE FROM ENSDF_cards WHERE 1")
    curs.execute("DELETE FROM ENSDF_lines WHERE 1")
    
    basedir = "/home/mpmendenhall/Documents/ENSDF/ensdf_150208/"
    fls = list(os.listdir(basedir))
    fls.sort()
    for f in fls:
        if f[:6] == "ensdf.":
            print(f)
            upload_cards(curs, open(basedir+f).read())

class ENSDFLine:
    """Uninterpreted line in ENSDF card"""
    def __init__(self, lid, mass, elem, rectp, txt):
        self.lid = lid
        self.mass = mass
        self.elem = elem
        self.rectp = rectp
        self.txt = txt
    def __repr__(self):
        nucid = "%3i%-2s"%(self.mass, self.elem)
        return nucid + self.rectp + self.txt
    
class ENSDFCardData:
    """Uninterpreted ENSDF card data"""
    
    def __init__(self, curs, cid):
        curs.execute("SELECT mass,elem FROM ENSDF_cards WHERE rowid = ?", (cid,))
        self.mass, self.elem = curs.fetchone()
        curs.execute("SELECT rowid,mass,elem,rectp,txt FROM ENSDF_lines WHERE card = ? ORDER BY rowid", (cid,))
        self.lines = [ENSDFLine(*r) for r in curs.fetchall()]
    
    def to_text(self):
        """Reconstitute original text representation"""
        return "\n".join([str(l) for l in self.lines])
    
if __name__ == "__main__":
    datdir = "/home/mpmendenhall/Documents/ENSDF/"
    conn = sqlite3.connect(datdir+"ENSDF.db")
    curs = conn.cursor()
    
    if True:
        upload_dataset(curs,datdir+"ensdf_150208/")
        conn.commit()
    else:
        curs.execute('select rowid from ENSDF_cards WHERE mass = 12 AND elem = "C"')
        for cid in curs.fetchall():
            print("\n"+"-"*80)
            print(ENSDFCardData(curs,cid[0]).to_text())
    
    conn.close()
    