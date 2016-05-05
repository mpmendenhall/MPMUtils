#!/usr/bin/python3
"""@package AtomsDB
Atom information resources --- currently dumb elements list"""

import sqlite3

class ElementNames:
    """Element name/symbol lookup table"""
    def __init__(self, fname="ElementsList.txt"):
        # (Z,symb,name)
        self.dat = [(int(x[0]),x[1],x[2]) for x in [l.split() for l in open(fname).readlines()] if len(x)==3]
        self.byNum = dict([(e[0],e) for e in self.dat])
        self.bySymb = dict([(e[1].lower(),e) for e in self.dat])
        self.byName = dict([(e[2].lower(),e) for e in self.dat])
        
    def elNum(self, s):
        """Element Z by name or symbol, case-insensitive"""
        s = s.lower()
        n = self.bySymb.get(s,None)
        n = n if n else self.byName.get(s,None)
        return n[0] if n else None
    
    def elSym(self,Z):
        """Element symbol by Z"""
        return self.byNum.get(Z,(None,None,None))[1]

def parse_NIST_Isotopes_value(s):
    """Parse range or uncertainty values from NIST isotopes text"""
    s = s.replace("#","").strip("[])")
    try: return float(s),None
    except: pass

    if ',' in s:
        s = [float(x) for x in s.split(",")]
        return 0.5*(s[0]+s[1]), 0.5*(s[1]-s[0])
    elif "(" in s:
        s = s.split("(")
        dvs = list(s[0])
        i = len(dvs)-1
        j = len(s[1])-1
        while i >= 0:
            if dvs[i].isdigit():
                dvs[i] = s[1][j] if j >= 0 else '0'
                j -= 1
            i -= 1
        return float(s[0]),float(''.join(dvs))
    #assert not s
    return None,None

def fill_Isotopes_DB(curs):
    isotdat = []
    atwt = { }
    for i in open("NIST_Isotopes.txt").read().split("\n\n"):
        try:
            i = [l.split("=")[1].strip() for l in i.split("\n")]
            Z = int(i[0])
            sym = i[1]
            A = int(i[2])
            m,dm = parse_NIST_Isotopes_value(i[3])
            f,df = parse_NIST_Isotopes_value(i[4])
            aw,daw = parse_NIST_Isotopes_value(i[5])
            isotdat.append((Z,A,sym,m,dm,f,df))
            if Z in atwt: assert atwt[Z] == (aw,daw)
            else: atwt[Z] = (aw,daw)
        except:
            print(i)
            
    curs.executemany("INSERT INTO Isotopes VALUES (?,?,?,?,?,?,?)", isotdat)
    
    eldat = []
    E = ElementNames()
    for Z in E.byNum:
        eldat.append((Z,E.elSym(Z),E.byNum[Z][2])+atwt.get(Z,(None,None)))
    curs.executemany("INSERT INTO Elements VALUES (?,?,?,?,?)", eldat)
    
if __name__=="__main__":
    conn = sqlite3.connect("Isotopes.db")
    curs = conn.cursor()
    fill_Isotopes_DB(curs)
    curs.execute("analyze")
    conn.commit()
    