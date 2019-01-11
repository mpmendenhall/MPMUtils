#! /bin/env python3

from NucCanvas import *
from ENDF6_DB import *
from argparse import ArgumentParser

def FPyields(sids, EDB, E = None):
    """Plot fission product yields distribution"""

    DCs = []
    for sid in sids:
        sd = EDB.get_section(sid)
        if E is None: E = sd.products[0].E
        print("Fission yields for", sd.Z, sd.A, E)
        prods = sd.eval_FPY(E)
        wmax = 0
        DC = DecaySet()

        for n in prods:
            w = prods[n][0]
            wmax = max(w,wmax)
            if w:
                n = NucState(n[1], n[0], n[2], w)
                if sid != sids[0]:
                    n.baseColor = rgb.blue
                    n.dx = n.dy = 0.05
                DC.addState(n)

        for k in DC.states:
            w = DC.states[k].w
            DC.states[k].w = max(1 + log(w/wmax)/log(1e3), 0)

        DCs.append(DC)

    NC = NucCanvas()
    NC.Acondense = {0:0}
    for DC in DCs: DC.draw(NC)
    return NC

def make_dplot(sid, EDB, outname = None):

    def dchain(sid, EDB, c, w = 1):
        sd = EDB.get_section(sid)
        n = NucState(sd.A, sd.Z, sd.LISO, w)
        n.HL = sd.T_h
        c.addState(n)

        if sd.NST: return sd

        for b in sd.branches.data:

            ZA = (sd.Z, sd.A)
            for n,r in enumerate(b.RTYP):

                Z,A = ZA
                if   r == 1: c.addTrans(BetaTrans(A, Z, w*b.BR))
                elif r == 2: c.addTrans(BetaTrans(A, -Z, w*b.BR))
                elif r == 3: c.addTrans(ITrans(A, Z, w*b.BR), True)
                elif r == 4: c.addTrans(AlphaTrans(A, Z, w*b.BR))
                elif r == 5: c.addTrans(NPTrans(A, Z, w*b.BR))
                elif r == 6: c.addTrans(SFTrans(A, Z, w*b.BR), True)
                elif r == 7: c.addTrans(NPTrans(A, -Z, w*b.BR))

                # transitioning out of intermediate state in multi-component transition
                if n:
                    ns = NucState(A, Z, 0.9, w*b.BR)
                    ns.HL = 1e-18
                    c.addState(ns)

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
                n = NucState(A, Z, 0, w*b.BR)
                c.addState(n)

        return sd


    DS = DecaySet()
    sd = dchain(sid, EDB, DS)
    print("\nGenerated decay for", sd.Z, sd.A)
    if sd.NST: return None # skip plotting stable

    NC = NucCanvas()
    DS.draw(NC)

    if outname is None:
        outname = "%03i%s%03i"%(sd.A, NC.elnames.elSym(sd.Z), sd.Z)
        if sd.LIS: outname += "_%i"%sd.LIS
        outname += "_Decay.pdf"
    if not outname: return NC
    NC.writetofile(outname)
    return None

def IsoTable(EDB, withDK = True):
    sids = EDB.find_sections({"MF": 8, "MT": 457})
    DS = DecaySet()
    for s in sids:
        sd = EDB.get_section(s)
        if sd.LISO: continue
        n = NucState(sd.A, sd.Z, sd.LISO, 1)
        n.frameStyle.append(style.linewidth.THick if sd.T_h else style.linewidth.THICk)
        n.HL = sd.T_h
        DS.addState(n)
        if withDK:
            for b in sd.branches.data:
                ZA = (sd.Z, sd.A)
                for n,r in enumerate(b.RTYP):

                    Z,A = ZA
                    if   r == 1: DS.addTrans(BetaTrans(A, Z, b.BR))
                    elif r == 2: DS.addTrans(BetaTrans(A, -Z, b.BR))
                    elif r == 3: DS.addTrans(ITrans(A, Z, b.BR), True)
                    elif r == 4: DS.addTrans(AlphaTrans(A, Z, b.BR))
                    elif r == 5: DS.addTrans(NPTrans(A, Z, b.BR))
                    elif r == 6: DS.addTrans(SFTrans(A, Z, b.BR), True)
                    elif r == 7: DS.addTrans(NPTrans(A, -Z, b.BR))

                    ZA = File8_DecayBranch.deltaZA(r, Z, A)
                    if ZA != (Z,A): break

    print("Rendering graphic...")
    NC = NucCanvas()
    NC.dA[0] = 1
    DS.draw(NC)
    NC.HLkey(2.5, 3)
    return NC

if __name__ == "__main__":
    parser = ArgumentParser()
    parser.add_argument("--db",     help="location of DB file")
    parser.add_argument("--A",      type=int,   help="filter by A")
    parser.add_argument("--Z",      type=int,   help="filter by Z")
    parser.add_argument("--every",  action="store_true", help="plot EVERYTHING!")
    parser.add_argument("--isots",  action="store_true", help="table of the isotopes")
    parser.add_argument("--out",    help="output filename (blank for auto)")
    parser.add_argument("--FP",     action="store_true", help="fission products distribution")
    options = parser.parse_args()

    EDB = ENDFDB(options.db)

    if options.FP:
        if options.out is None: options.out = "FissProds.pdf"
        d = document.document()
        sids4 = EDB.find_sections({"A": options.A, "Z": options.Z, "MF": 8, "MT": 454})
        sids9 = EDB.find_sections({"A": options.A, "Z": options.Z, "MF": 8, "MT": 459})
        assert len(sids4) == len(sids9)
        for i in range(len(sids4)):
            c = FPyields((sids4[i],sids9[i]), EDB)
            if c is not None: d.append(document.page(c))
        d.writetofile(options.out)
        exit(0)

    if options.isots:
        IsoTable(EDB).writetofile(options.out if options.out else "TableOfTheIsotopes.pdf")
        exit(0)

    if options.Z is None and options.A is None and not options.every: exit(0)

    sids = EDB.find_sections({"A": options.A, "Z": options.Z, "MF": 8, "MT": 457})
    d = document.document()

    for s in sids:
        p = make_dplot(s, EDB, 0 if len(sids) > 1 else options.out)
        if p is not None: d.append(document.page(p))

    if options.out is None: options.out = "all_decays.pdf" if options.every else "decays.pdf"
    if len(sids) > 1:
        NC = NucCanvas()
        NC.HLkey()
        d.append(document.page(NC))
        print("Outputting", options.out)
        d.writetofile(options.out)
