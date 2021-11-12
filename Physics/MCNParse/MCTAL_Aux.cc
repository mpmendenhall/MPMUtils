/// \file MCTAL_Aux.cc

#include "MCTAL_Aux.hh"
#include <stdio.h>
#include "to_str.hh"

void MCTAL_TFC::load(lineReader& i) {
    string s_tmp;
    int n_tf;
    i.next() >> s_tmp >> n_tf;
    check_expected(upper(s_tmp), "TFC");

    for(auto& j: j_tf) i >> j;
    while(n_tf--) {
        s_tfc f;
        i.next() >> f.nps >> f.tally >> f.err >> f.fom;
        push_back(f);
    }
}

void MCTAL_TFC::display() const {
    printf("Tally Fluctuation Table with %zu entries\n", size());
}

void MCTAL_KCODE::load(lineReader& i) {
    string s_tmp;
    i.next() >> s_tmp  >> n_cyc >> n_scyc >> n_var;
    check_expected(upper(s_tmp), "KCODE");

    if(n_var == 0 || n_var == 5) {
        for(int n=0; n < n_cyc; ++n) {
            s_kcyc c;
            i.next() >> c.keff1 >> c.keff2 >> c.keff3 >> c.rl1 >> c.rl2;
            push_back(c);
        }
    } else if(n_var == 19) {
        throw std::logic_error("TODO implement 19-element KCODE parsing");
    } else {
        throw std::runtime_error("Bad number of KCODE vars: " + to_str(n_var));
    }
}

void MCTAL_KCODE::display() const {
    printf("KCODE with %i variables for %i code cycles, %i settle cycles\n",
           n_var, n_cyc, n_scyc);
}
