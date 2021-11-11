/// \file MCTAL_Axis.cc

#include "MCTAL_Axis.hh"
#include <stdio.h>

void MCTAL_Axis::load(const string& cs, istream& i, bool to_endline) {
    char _c;
    i.get(_c);
    check_expected(::toupper(_c), cs);
    i.get(_c);
    _c = ::toupper(_c);
    check_expected(_c," TC");
    bintype = bintype_t(_c);
    i >> nbins;
    if(to_endline) i.ignore(256, '\n');
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

void MCTAL_AxBins::load(const string& cs, istream& i) {

    MCTAL_Axis::load(cs, i, false);

    // TODO check for 'f' being present
    int f = 0;
    //i >> f;
    is_bin_lowedge = !f;

    i.ignore(256, '\n');
    if(nbins) {
        resize(nbins - (bintype == BINTP_TOTAL? 1 : 0));
        for(auto& x: *this) i >> x;
        i.ignore(256, '\n');
    }
}

void MCTAL_IntAx::load(istream& i) {
    MCTAL_Axis::load("F", i);
    if(bintype != BINTP_NONE)
        throw std::runtime_error("Invalid bin type for F");
    if(nbins) {
        resize(nbins);
        for(auto& x: *this) i >> x;
        i.ignore(256, '\n');
    }
}
