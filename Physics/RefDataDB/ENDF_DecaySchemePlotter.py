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

            if b.RTYP == (1,): NucCanvas.addwt(c.betas, (sd.A,sd.Z), w*b.BR)
            if b.RTYP == (4,): NucCanvas.addwt(c.alphas, (sd.A,sd.Z), w*b.BR)

            ss = EDB.find_F8MT457(A, Z, b.RFS)
            if ss: dchain(ss, EDB, c, w*b.BR)
            else:
                print("Missing", A, Z, b.RFS)
                print(sd)
                print(b)

        return sd

    NC = NucCanvas()
    sd = dchain(sid, EDB, NC)
    NC.drawNucs()
    NC.c.writePDFfile("%i%s%i_Decay"%(sd.A, NC.elnames.elSym(sd.Z), sd.Z))


if __name__ == "__main__":
    parser = ArgumentParser()
    parser.add_argument("--db",     help="location of DB file")
    parser.add_argument("--A",      type=int,   help="filter by A")
    parser.add_argument("--Z",      type=int,   help="filter by Z")
    options = parser.parse_args()

    EDB = ENDFDB(options.db)

    sids = EDB.find_sections({"A": options.A, "Z": options.Z, "MF": 8, "MT": 457})
    assert len(sids) == 1
    make_dplot(sids[0], EDB)
