/// \file MCTAL_Axis.cc

#include "MCTAL_Axis.hh"
#include <stdio.h>

void MCTAL_Axis::load(const string& cs, lineReader& i) {
    char _c;
    i.next().get(_c);
    check_expected(::toupper(_c), cs);
    i.get(_c);
    _c = ::toupper(_c);
    check_expected(_c," TC");
    bintype = bintype_t(_c);
    i >> nbins;
}

void MCTAL_Axis::display() const {
    printf("%s Axis for %i %s%s\n", title.c_str(), nbins,
           is_bin_lowedge? "bins" : "positions",
           bintype == BINTP_NONE? "" : (bintype == BINTP_TOTAL?
           " including Total" : " Cumulative"));
}

vector<double> MCTAL_Axis::toVec() const {
    vector<double> v(nvals());
    for(size_t i = 0; i < v.size(); ++i) v[i] = (*this)(i);
    return v;
}

void MCTAL_Axis::showbins() const {
    printf("{ ");
    for(auto i: toVec()) printf("%g ", i);
    printf("}");
}

void MCTAL_AxBins::load(const string& cs, lineReader& i) {

    MCTAL_Axis::load(cs, i);

    // TODO check for 'f' being present
    int f = 0;
    i >> f;
    is_bin_lowedge = !f;

    if(nbins) {
        resize(nbins - (bintype == BINTP_TOTAL? 1 : 0));
        for(auto& x: *this) {
            i.checkEnd();
            i >> x;
        }
    }
}

void MCTAL_IntAx::load(lineReader& i) {
    MCTAL_Axis::load("F", i);
    if(bintype != BINTP_NONE) throw std::runtime_error("Invalid bin type for F");
    if(nbins) {
        resize(nbins);
        for(auto& x: *this) {
            i.checkEnd();
            i >> x;
        }
    }
    display();
}
