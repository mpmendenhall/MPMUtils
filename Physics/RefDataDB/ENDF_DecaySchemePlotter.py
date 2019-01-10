#! /bin/env python3

from NucCanvas import NucCanvas
from ENDF6_DB import *
from argparse import ArgumentParser

def make_dplot(sid, EDB):

    def dchain(sid, EDB, c, w = 1):
        sd = EDB.get_section(sid)
        NucCanvas.addwt(c.nucs, (sd.A, sd.Z), w)

        if sd.NST: return sd

        for b in sd.branches.data:
            ZA = b.daughterZA(sd.Z, sd.A)
            if not ZA: continue
            Z,A = ZA
            assert A != sd.A or Z != sd.Z or b.RFS != sd.LISO

            A0, Z0 = sd.A, sd.Z
            for r in b.RTYP:
                if r in (0,8,9): pass
                elif r == 1:
                    NucCanvas.addwt(c.betas, (A0, Z0), w*b.BR)
                    Z0 += 1
                elif r == 2:
                    NucCanvas.addwt(c.betas, (A0,-Z0), w*b.BR)
                    Z0 -= 1
                elif r == 4:
                    NucCanvas.addwt(c.alphas,(A0, Z0), w*b.BR)
                    A0 -= 4; Z0 -= 2
                elif r == 5:
                    NucCanvas.addwt(c.nps,   (A0, Z0), w*b.BR)
                    A0 -= 1
                elif r == 7:
                    NucCanvas.addwt(c.nps,   (A0,-Z0), w*b.BR)
                    A0 -= 1; Z0 -= 1
                else: break

            ss = EDB.find_F8MT457(A, Z, b.RFS)
            if ss: dchain(ss, EDB, c, w*b.BR)
            else:
                print("Missing", A, Z, b.RFS)
                print(sd)
                print(b)

        return sd

    NC = NucCanvas()
    sd = dchain(sid, EDB, NC)
    NC.condense()
    NC.drawNucs()
    dname = "%03i%s%03i"%(sd.A, NC.elnames.elSym(sd.Z), sd.Z)
    if sd.LIS: dname += "_%i"%sd.LIS
    NC.c.writePDFfile(dname + "_Decay")


if __name__ == "__main__":
    parser = ArgumentParser()
    parser.add_argument("--db",     help="location of DB file")
    parser.add_argument("--A",      type=int,   help="filter by A")
    parser.add_argument("--Z",      type=int,   help="filter by Z")
    parser.add_argument("--lvl",    type=int,   help="filter by level")
    options = parser.parse_args()

    EDB = ENDFDB(options.db)

    sids = EDB.find_sections({"A": options.A, "Z": options.Z, "MF": 8, "MT": 457})
    for s in sids: make_dplot(s, EDB)
