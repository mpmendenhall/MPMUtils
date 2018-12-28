#!/bin/env python

from ENDF_Reader import *
from argparse import ArgumentParser

if __name__=="__main__":
    parser = ArgumentParser()
    parser.add_argument("--file",   help="input file")
    options = parser.parse_args()

    if not options.file: exit(0)

    f = open(options.file,"r")
    print(load_ENDF_Section(f))
