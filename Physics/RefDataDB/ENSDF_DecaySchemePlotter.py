#! /bin/env python3

from NucCanvas import *
from ENSDF_Reader import *
from argparse import ArgumentParser

def IsoTable(EDB):
    DS = DecaySet()
    EDB.curs.execute("SELECT entry_id FROM ENSDF_entries WHERE DSID LIKE ?", ("ADOPTED LEVELS%",))
    for s in [r[0] for r in EDB.curs.fetchall()]:
        sd = EDB.get_entry(s)
        if not sd.levels:
            print("No level information found for")
            print(sd)
            continue
        n = NucState(sd.A, sd.Z, 0, 1)
        n.HL = sd.levels[0].T
        if n.HL == float("inf"): n.HL = None
        elif type(n.HL) != type(1.0): n.HL = 1e-18
        n.frameStyle.append(style.linewidth.THick if n.HL else style.linewidth.THICk)
        DS.addState(n)

    print("Rendering graphic...")
    NC = NucCanvas()
    NC.dA[0] = 1
    NC.dA[1] -= 0.015
    DS.draw(NC)
    NC.HLkey(2.5, 3)
    return NC

if __name__ == "__main__":
    parser = ArgumentParser()
    parser.add_argument("--db",     help="location of DB file")
    parser.add_argument("--isots",  action="store_true", help="table of the isotopes")
    parser.add_argument("--out",    help="output filename (blank for auto)")
    options = parser.parse_args()

    EDB = ENSDFDB(options.db)

    if options.isots:
        IsoTable(EDB).writetofile(options.out if options.out else "TableOfTheIsotopes_ENSDF.pdf")
        exit(0)
