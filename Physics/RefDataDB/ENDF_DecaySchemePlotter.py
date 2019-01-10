#! /bin/env python3

from NucCanvas import NucCanvas
from ENDF6_DB import *
from argparse import ArgumentParser

def make_dplot(sid, EDB, outname = None):

    def dchain(sid, EDB, c, w = 1):
        sd = EDB.get_section(sid)
        NucCanvas.addwt(c.nucs, (sd.A, sd.Z, sd.LISO), w)

        if sd.NST: return sd

        for b in sd.branches.data:

            A0, Z0 = sd.A, sd.Z
            for n,r in enumerate(b.RTYP):
                if r == 3:
                    NucCanvas.addwt(c.its, (A0, Z0), w*b.BR) # gammas, no change to A,Z
                    continue
                elif r == 1: NucCanvas.addwt(c.betas, (A0, Z0), w*b.BR)
                elif r == 2: NucCanvas.addwt(c.betas, (A0,-Z0), w*b.BR)
                elif r == 4: NucCanvas.addwt(c.alphas,(A0, Z0), w*b.BR)
                elif r == 5: NucCanvas.addwt(c.nps,   (A0, Z0), w*b.BR)
                elif r == 6: NucCanvas.addwt(c.SFs,   (A0, Z0), w*b.BR); break
                elif r == 7: NucCanvas.addwt(c.nps,   (A0,-Z0), w*b.BR)
                else:
                    print("Unhandled transition type", r)
                    break

                if (A0,Z0) != (sd.A, sd.Z): # intermediate transition
                    NucCanvas.addwt(c.nucs, (A0, Z0), w*b.BR)

                if r == 1: Z0 += 1
                elif r == 2: Z0 -= 1
                elif r == 4: A0 -= 4; Z0 -= 2
                elif r == 5: A0 -= 1
                elif r == 7: A0 -= 1; Z0 -= 1


            ZA = b.daughterZA(sd.Z, sd.A)
            if not ZA: continue
            Z,A = ZA
            assert A != sd.A or Z != sd.Z or b.RFS != sd.LISO

            ss = EDB.find_F8MT457(A, Z, b.RFS)
            if ss: dchain(ss, EDB, c, w*b.BR)
            else:
                print("Missing", A, Z, b.RFS)
                print(sd)
                print(b)
                NucCanvas.addwt(c.nucs, (A, Z), w*b.BR)

        return sd

    NC = NucCanvas()
    sd = dchain(sid, EDB, NC)
    NC.condense()
    NC.drawNucs()

    if outname is None:
        outname = "%03i%s%03i"%(sd.A, NC.elnames.elSym(sd.Z), sd.Z)
        if sd.LIS: outname += "_%i"%sd.LIS
        outname += "_Decay"
    NC.c.writePDFfile(outname)

if __name__ == "__main__":
    parser = ArgumentParser()
    parser.add_argument("--db",     help="location of DB file")
    parser.add_argument("--A",      type=int,   help="filter by A")
    parser.add_argument("--Z",      type=int,   help="filter by Z")
    parser.add_argument("--lvl",    type=int,   help="filter by level")
    parser.add_argument("--every",  action="store_true", help="plot EVERYTHING!")
    parser.add_argument("--out",    help="output filename (blank for auto)")
    options = parser.parse_args()

    EDB = ENDFDB(options.db)

    if options.every:
        options.Z = 999
        os.system("rm *_Decay.pdf")
        if options.out is None: options.out = "decays.pdf"
        for Z in range(120):
            sids = EDB.find_sections({"Z": Z, "MF": 8, "MT": 457})
            if not sids: continue
            for s in sids: make_dplot(s, EDB)
            os.system("pdfunite *_Decay.pdf %03i_decays.pdf"%Z)
            os.system("rm *_Decay.pdf")
        os.system("pdfunite ???_decays.pdf "+options.out)


    sids = EDB.find_sections({"A": options.A, "Z": options.Z, "MF": 8, "MT": 457})
    for s in sids: make_dplot(s, EDB, options.out)
