#!/usr/bin/python3
"""@package ENSDF_CardDB
Database interface for storing and retrieving ENSDF (uninterpreted) card information"""

import sqlite3
import os

def upload_card(curs, lines):
    """Store one card, split into lines, to DB"""
    if not lines: return
    print(lines[0])
    
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
    lnum = 0
    for l in lines:
        if not l.strip(): continue
        ldata.append((cid,lnum,l[5:8],l[8:80]))
        lnum += 1
    curs.executemany("INSERT INTO ENSDF_lines VALUES (?,?,?,?)", ldata)
  
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

def card_text(curs,cid):
    """Reconstitute card text for card ID number"""
    curs.execute("SELECT mass,elem FROM ENSDF_cards WHERE rowid = ?", (cid,))
    nucid = "%3i%-2s"%curs.fetchone()
    curs.execute("SELECT rectp,text FROM ENSDF_lines WHERE card = ? ORDER BY n", (cid,))
    return "\n".join([nucid+l[0]+l[1] for l in curs.fetchall()])

def upload_dataset(curs,basedir):
    """Upload entire dataset from directory"""
    curs.execute("DELETE FROM ENSDF_cards WHERE 1")
    curs.execute("DELETE FROM ENSDF_lines WHERE 1")
    
    basedir = "/home/mpmendenhall/Documents/ENSDF/ensdf_150208/"
    for f in os.listdir(basedir):
        if f[:6] == "ensdf.": upload_cards(curs, open(basedir+f).read())
        
class CardData:
    """Uninterpreted ENSDF card data"""
    
    def __init__(self, curs, cid):
        curs.execute("SELECT mass,elem FROM ENSDF_cards WHERE rowid = ?", (cid,))
        self.mass, self.elem = curs.fetchone()
        curs.execute("SELECT rectp,text FROM ENSDF_lines WHERE card = ? ORDER BY n", (cid,))
        self.lines = curs.fetchall()
    
    def to_text(self):
        """Reconstitute original text representation"""
        nucid = "%3i%-2s"%(self.mass, self.elem)
        return "\n".join([nucid+l[0]+l[1] for l in self.lines])
    
if __name__ == "__main__":
    datdir = "/home/mpmendenhall/Documents/ENSDF/"
    conn = sqlite3.connect(datdir+"ENSDF.db")
    curs = conn.cursor()
    #upload_dataset(curs,datdir+"ensdf_150208/")
    
    curs.execute('select rowid from ENSDF_cards WHERE mass = 12 AND elem = "C"')
    for cid in curs.fetchall():
        print("\n"+"-"*80)
        print(CardData(curs,cid[0]).to_text())
    
    #conn.commit()
    conn.close()
    