/// \file MCTAL_Header.cc

#include "MCTAL_Header.hh"
#include <stdio.h>
#include <inttypes.h>

MCTAL_Header::MCTAL_Header(istream& i) {
    i >> kod >> ver >> prob_date >> prob_time >> knod >> nps >> rnr;
    i.ignore(256, '\n');
    char _c;
    i.get(_c);
    check_expected(_c, " ");
    getline(i, probid);
    string s_tmp;
    i >> s_tmp;
    check_expected(upper(s_tmp), "NTAL");
    i >> ntal;
    // TODO handle NPERT if present
    i.ignore(256, '\n');
    tallynums.resize(ntal);
    for(int n=0; n < ntal; ++n) i >> tallynums[n];
    i.ignore(256, '\n');
}

void MCTAL_Header::display() const {
    printf("MCTAL File from %s version %s, on %s at %s: '%s'\n"
            "\tdump %i, %" PRId64 " particles using %" PRId64 " quasirandom numbers\n"
            "\tcontaining %i tallies and %i perturbations.\n",
           kod.c_str(), ver.c_str(), prob_date.c_str(), prob_time.c_str(),
           probid.c_str(), knod, nps, rnr, ntal, npert);
}
