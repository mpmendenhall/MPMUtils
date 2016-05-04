#!/usr/bin/python3
"""@package ENSDF_CardDB
Database interface for storing and retrieving ENSDF (uninterpreted) card information"""

import sqlite3
import os

class CardHeader:
    """Parser for first line identification in card"""
    def __init__(self,l):
        self.mass = int(l[:3])
        self.elem = l[3:5].strip()
        self.DSID = l[9:39].strip()
        self.DSREF = l[39:65].strip()
        self.PUB = l[65:74].strip()
        self.EDATE = int(l[74:80]) if l[74:80].strip() else 0
    def upload(self,curs):
        curs.execute("INSERT INTO ENSDF_cards VALUES (?,?,?,?,?,?)", (self.mass,self.elem,self.DSID,self.DSREF,self.PUB,self.EDATE))
        return curs.lastrowid 

def upload_card(curs, lines):
    """Store one card, split into lines, to DB"""
    if not lines: return
    #print(lines[0])
    
    h = CardHeader(lines[0])
    cid = h.upload(curs)
    
    ldata = []
    for l in lines:
        if not l.strip(): continue
        try:
            ldata.append((cid, int(l[:3]), l[3:5].strip(), l[5:8], l[8:80]))
        except:
            print("Failed line: '%s'"%l)
            raise
    curs.executemany("INSERT INTO ENSDF_lines VALUES (?,?,?,?,?)", ldata)
    
def identify_update(curs, lines):
    """Identify previous card in DB for which this set of lines seems to be an update"""
    if not lines: return None
    h = CardHeader(lines[0])
    curs.execute("SELECT rowid FROM ENSDF_cards WHERE mass = ? AND elem = ? AND DSID LIKE ?", (h.mass,h.elem,h.DSID))
    return [r[0] for r in curs.fetchall()]

def update_card(curs, lines):
    if not lines: return
    cids = identify_update(curs,lines)
    if not cids:
        print("No prior entries for",lines[0])
        upload_card(curs,lines)
    elif len(cids) > 1:
        print("*** Multiple matches for",lines[0])
    else:
        cid = cids[0]
        print("Updating",lines[0],"over",cid)
        curs.execute("DELETE FROM ENSDF_cards WHERE rowid = ?", (cid,))
        upload_card(curs,lines)
    
def upload_cards(curs, txt, upd = False):
    """Upload or update many cards split from file"""
    lns = []
    for l in txt.split("\n"): 
        if not l.strip():
            if upd: update_card(curs,lns)
            else: upload_card(curs,lns)
            lns = []
        else:
            lns.append(l)
    if upd: update_card(curs,lns)
    else: upload_card(curs,lns)
    

def upload_dataset(curs, basedir, upd=False):
    """Upload/update entire dataset from directory"""
    if not upd:
        curs.execute("DELETE FROM ENSDF_cards WHERE 1")
        curs.execute("DELETE FROM ENSDF_lines WHERE 1")
    
    fls = list(os.listdir(basedir))
    fls.sort()
    for f in fls:
        if upd or f[:6] == "ensdf.":
            #print(f)
            upload_cards(curs, open(basedir+f).read(), upd)
            
    if upd: # sanitize removed records
        print("Deleting removed cards...")
        curs.execute("DELETE FROM ENSDF_lines WHERE card NOT IN (SELECT rowid FROM ENSDF_cards)")

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
        #upload_dataset(curs,datdir+"ensdf_150208/")
        upload_dataset(curs,datdir+"Updates_20160504/",True)
        #upload_cards(curs, open(datdir+"ensdf_150208/ensdf_151208.upd").read())
        conn.commit()
    else:
        curs.execute('select rowid from ENSDF_cards WHERE mass = 12 AND elem = "C"')
        for cid in curs.fetchall():
            print("\n"+"-"*80)
            print(ENSDFCardData(curs,cid[0]).to_text())
    
    conn.close()
    