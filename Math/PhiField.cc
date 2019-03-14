/// \file PhiField.cc

#include "PhiField.hh"

//const PhiField PhiField::one{{1},{}};
//const PhiField PhiField::phi{{},{1}};

std::ostream& operator<<(std::ostream& o, const PhiField& r) {
    if(r.a && r.b) o << "(";

    if(!r) o << "0";

    if(r.a) o << r.a;
    auto& q = r.b;
    if(q) {
        if(r.a) o << " ";
        auto nd = q.components();
        if(abs(nd.first) == 1) o << (q.positive? "+" : "-");
        else o << nd.first;
        o << "φ";
        if(nd.second > 1) o << "/" << nd.second;
    }
    if(r.a && r.b) o << ")";
    return o;
}
