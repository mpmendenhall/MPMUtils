/// \file MCTAL_Header.cc

#include "MCTAL_Header.hh"
#include <stdio.h>
#include <inttypes.h>

MCTAL_Header::MCTAL_Header(lineReader& i) {
    try {
        i.next() >> kod >> ver >> prob_date >> prob_time >> knod >> nps >> rnr;

        probid = i.next().lstr;
        check_expected(probid.at(0), " ");

        string s_tmp;
        i.next() >> s_tmp >> ntal;
        check_expected(upper(s_tmp), "NTAL");
        // TODO handle NPERT if presterminate called after throwing an instance of 'std::runtime_error'ent

        i.next();
        tallynums.resize(ntal);
        for(int n=0; n < ntal; ++n) i >> tallynums[n];
    } catch(std::runtime_error& e) {
        printf("Problem parsing MCTAL header at line %i [%s]\n", i.lno, i.lstr.c_str());
        throw;
    }
}

void MCTAL_Header::display() const {
    printf("MCTAL File from %s version %s, on %s at %s: '%s'\n"
            "\tdump %i, %" PRId64 " particles using %" PRId64 " quasirandom numbers\n"
            "\tcontaining %i tallies and %i perturbations.\n",
           kod.c_str(), ver.c_str(), prob_date.c_str(), prob_time.c_str(),
           probid.c_str(), knod, nps, rnr, ntal, npert);
}
