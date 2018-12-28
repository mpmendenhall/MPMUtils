#!/bin/env python

from ENDF_Reader import *
from argparse import ArgumentParser

if __name__=="__main__":
    parser = ArgumentParser()
    parser.add_argument("--file",   help="input file")
    options = parser.parse_args()

    if not options.file: exit(0)

    f = open(options.file,"r")

    # tape header line
    h0 = ENDF_Record(next(f))
    assert h0.MF == h0.MT == 0 and h0.MAT == 1

    while f:
        s = load_ENDF_Section(f)
        print("\n--------------------------------------")
        print(s)
        if s.rectp == "TEND": break

