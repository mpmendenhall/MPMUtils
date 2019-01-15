#! /bin/env python3

from NucCanvas import *
from ENSDF_Reader import *
from argparse import ArgumentParser

def IsoTable(EDB):
    DS = DecaySet()
    EDB.curs.execute("SELECT entry_id FROM ENSDF_entries WHERE DSID LIKE ?", ("ADOPTED LEVELS%",))
    for s in [r[0] for r in EDB.curs.fetchall()]:
        sd = EDB.get_entry(s)
        if not sd.levels or sd.levels[0].E is None: continue
        n = NucState(sd.A, sd.Z, 0, 1)
        n.HL = sd.levels[0].T
        if type(n.HL) == type("abc") and n.HL[-2:] == 'EV': n.HL = 1e-18
        n.frameStyle.append(style.linewidth.THICk if n.HL == float("inf") else style.linewidth.THick)
        assert n.idx not in DS.states
        DS.addState(n)

    print("Rendering graphic...")
    NC = NucCanvas()
    NC.dA[0] = 1
    NC.dA[1] -= 0.015
    DS.draw(NC)
    NC.HLkey(2.5, 3)
    return NC

def dchain(EDB, Pid):
    sd = EDB.get_entry(Pid)
    EDB.curs.execute("SELECT child_id, E, T FROM Decay_Index WHERE parent_id = ?", (Pid,))
    s = "" #sd.printid()
    for r in EDB.curs.fetchall():
        c = EDB.get_entry(r[0])
        s += "%s\t(%s,\t%s) ->\n"%(c.printid(), str(r[1]),str(r[2]))
        a = EDB.adopted_for_entry(r[0])
        if a == Pid: continue
        s += indent(dchain(EDB, a),'\t')+'\n'
    return s

if __name__ == "__main__":
    parser = ArgumentParser()
    parser.add_argument("--db",     help="location of DB file")
    parser.add_argument("--isots",  action="store_true", help="table of the isotopes")
    parser.add_argument("--out",    help="output filename (blank for auto)")
    parser.add_argument("--A",      type=int,   help="filter by A")
    parser.add_argument("--Z",      type=int,   help="filter by Z")
    options = parser.parse_args()

    EDB = ENSDFDB(options.db)

    if options.A and options.Z:
        print(dchain(EDB, EDB.find_adopted(options.A,options.Z)))

    if options.isots:
        IsoTable(EDB).writetofile(options.out if options.out else "TableOfTheIsotopes_ENSDF.pdf")
        exit(0)
