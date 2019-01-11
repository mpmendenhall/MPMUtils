#! /bin/env python3

from NucCanvas import *
from ENDF6_DB import *
from argparse import ArgumentParser

def FPyields(sid, EDB, E = None):
    """Plot fission product yields distribution"""
    sd = EDB.get_section(sid)
    if E is None: E = sd.products[0].E
    print("Fission yields for", sd.Z, sd.A, E)
    prods = sd.eval_FPY(E)
    NC = NucCanvas()
    wmax = 0
    for n in prods:
        w = prods[n][0]
        wmax = max(w,wmax)
        if w: NucCanvas.addwt(NC.nucs, (n[1], n[0], n[2]), w)
    for k in NC.nucs: NC.nucs[k] = max(1 + log(NC.nucs[k]/wmax)/log(1e3), 0)
    if not NC.nucs:
        print("\tNo fissions found!")
        print(sd)
        return None
    NC.condense()
    NC.drawNucs()
    return NC.c

def make_dplot(sid, EDB, outname = None):

    def dchain(sid, EDB, c, w = 1):
        sd = EDB.get_section(sid)
        n = (sd.A, sd.Z, sd.LISO)
        NucCanvas.addwt(c.nucs, n, w)
        if sd.T_h: c.HL[n] = sd.T_h

        if sd.NST: return sd

        for b in sd.branches.data:

            ZA = (sd.Z, sd.A)
            for n,r in enumerate(b.RTYP):

                Z,A = ZA
                if   r == 1: NucCanvas.addwt(c.betas, (A, Z), w*b.BR)
                elif r == 2: NucCanvas.addwt(c.betas, (A,-Z), w*b.BR)
                elif r == 3: NucCanvas.addwt(c.its,   (A, Z), w*b.BR)
                elif r == 4: NucCanvas.addwt(c.alphas,(A, Z), w*b.BR)
                elif r == 5: NucCanvas.addwt(c.nps,   (A, Z), w*b.BR)
                elif r == 6: NucCanvas.addwt(c.SFs,   (A, Z), w*b.BR)
                elif r == 7: NucCanvas.addwt(c.nps,   (A,-Z), w*b.BR)

                # transitioning out of intermediate state in multi-component transition
                if n:
                    NucCanvas.addwt(c.nucs, (A, Z, 0.9), w*b.BR)
                    c.HL[(A,Z,0.9)] = 1e-18

                ZA = File8_DecayBranch.deltaZA(r, Z, A)
                if ZA is None: break

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
    if sd.NST: return None # skip plotting stable
    print("\nGenerated decay for", sd.Z, sd.A)
    NC.condense()
    NC.drawNucs()

    if outname is None:
        outname = "%03i%s%03i"%(sd.A, NC.elnames.elSym(sd.Z), sd.Z)
        if sd.LIS: outname += "_%i"%sd.LIS
        outname += "_Decay.pdf"
    if not outname: return NC.c
    NC.c.writetofile(outname)
    return None

if __name__ == "__main__":
    parser = ArgumentParser()
    parser.add_argument("--db",     help="location of DB file")
    parser.add_argument("--A",      type=int,   help="filter by A")
    parser.add_argument("--Z",      type=int,   help="filter by Z")
    parser.add_argument("--every",  action="store_true", help="plot EVERYTHING!")
    parser.add_argument("--out",    help="output filename (blank for auto)")
    parser.add_argument("--FP",     action="store_true", help="fission products distribution")
    options = parser.parse_args()

    EDB = ENDFDB(options.db)

    if options.every:
        if options.out is None: options.out = "decays.pdf"

        d = document.document()
        d.append(document.page(NucCanvas.HLkey(None)))
        for Z in range(120):
            sids = EDB.find_sections({"Z": Z, "MF": 8, "MT": 457})
            if not sids: continue
            print("Generating z =",Z)

            for s in sids:
                p = make_dplot(s, EDB, 0)
                if p is not None: d.append(document.page(p))
        d.writetofile(options.out)
        exit(0)

    if options.FP:
        if options.out is None: options.out = "FissProds.pdf"
        d = document.document()
        for MT in (454,459):
            sids = EDB.find_sections({"A": options.A, "Z": options.Z, "MF": 8, "MT": MT})
            for s in sids:
                c = FPyields(s, EDB)
                if c is not None: d.append(document.page(c))
        d.writetofile(options.out)
        exit(0)

    sids = EDB.find_sections({"A": options.A, "Z": options.Z, "MF": 8, "MT": 457})
    d = document.document()
    for s in sids:
        p = make_dplot(s, EDB, 0 if len(sids) > 1 else options.out)
        if p is not None: d.append(document.page(p))
    if options.out is None: options.out = "decays.pdf"
    if len(sids) > 1:
        print("Outputting", options.out)
        d.writetofile(options.out)
