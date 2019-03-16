/// \file PhiField.cc

#include "PhiField.hh"

std::ostream& operator<<(std::ostream& o, const PhiField& r) {
    if(!r) return o << " 0";
    //if(r.a < 0 && 0 < r.b) return o << PhiField(0,r.b) << PhiField(r.a,0);

    if(r.a) o << (0 < r.a? "+" : "") << r.a;

    if(r.b) {
        auto nd = r.b.components();
        if(abs(nd.first) == 1) o << (r.b.positive? "+" : "-");
        else o << nd.first;
        o << "φ";
        if(nd.second > 1) o << "/" << nd.second;
    }

    return o;
}
