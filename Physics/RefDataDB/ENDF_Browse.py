#!/bin/env python3

# ./ENDF_Browse.py --browse ~/Data/ENDF-B-VII.1/

# rm $ENDFDB; sqlite3 $ENDFDB < ENDF6_DB_Schema.sql
# for f in ~/Data/ENDF-B-VII.1/*/*; do ./ENDF_Browse.py --load $f; done

from ENDF6_DB import *
from argparse import ArgumentParser


if __name__=="__main__":

    parser = ArgumentParser()
    parser.add_argument("--db",     help="location of DB file")
    parser.add_argument("--browse", help="summarize file contents to terminal")
    parser.add_argument("--load",   help="import input file to DB")
    parser.add_argument("--A",      type=int,   help="filter by A")
    parser.add_argument("--Z",      type=int,   help="filter by Z")
    parser.add_argument("--MF",     type=int,   help="filter by file number")
    parser.add_argument("--MT",     type=int,   help="filter by file section")
    parser.add_argument("--MAT",    type=int,   help="filter by material number")
    parser.add_argument("--display",action="store_true", help="Display located sections")
    parser.add_argument("--count",  action="store_true", help="Count entries matching query")
    parser.add_argument("--list",   action="store_true", help="Short-form listing of entries matching query")
    parser.add_argument("--fail",   action="store_true", help="Fail on unparsed data")

    options = parser.parse_args()

    if options.browse:
        f = open(options.browse,"r")
        while f:
            s = load_ENDF_Section(f)
            print("\n--------------------------------------")
            print(s)
            print("--------------------------------------\n")
            if s.rectp == "TEND": break

    EDB = ENDFDB(options.db)
    EDB.letFail = options.fail

    if options.load:

        print("Loading", options.load)
        f = open(options.load,"r")

        # tape header line
        h0 = ENDF_Record(next(f))
        assert h0.MF == h0.MT == 0 and h0.MAT == 1

        nloaded = 0
        while f:
            ls = pop_section_lines(f)
            h = ENDF_HEAD_Record(ls[0])
            if h.rectp == "TEND": break
            if not h.endlvl:
                sid = EDB.upload_section(h, ''.join(ls))
                nloaded += 1
                if options.display:
                    print("\n--------------------------------------")
                    print(EDB.get_section(sid))
                    print("--------------------------------------\n")

        EDB.conn.commit()
        print("\tLoaded", nloaded, "entries.")
        exit(0)

    sids = []
    if options.display or options.count or options.list:
        sids = EDB.find_sections({"A": options.A, "Z": options.Z, "MF": options.MF, "MT": options.MT, "MAT": options.MAT})
        print("Found", len(sids), "matching records.")
    if options.display:
        for s in sids:
            print("\n--------------------------------------")
            print(EDB.get_section(s))
    if options.list:
        for s in sids:
            s = EDB.get_section(s)
            print(s.printid() if s is not None else None)

