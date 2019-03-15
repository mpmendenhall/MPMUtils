/// \file PhiField.cc

#include "PhiField.hh"

//const PhiField PhiField::one{{1},{}};
//const PhiField PhiField::phi{{},{1}};

std::ostream& operator<<(std::ostream& o, const PhiField& r) {
    if(r.a != 0 && r.b != 0) o << "(";

    if(r == 0) o << "0";

    if(r.a != 0) o << r.a;
    auto& q = r.b;
    if(q != 0) {
        if(r.a != 0) o << " ";
        auto nd = q.components();
        if(abs(nd.first) == 1) o << (q.positive? "+" : "-");
        else o << nd.first;
        o << "φ";
        if(nd.second > 1) o << "/" << nd.second;
    }
    if(r.a != 0 && r.b != 0) o << ")";
    return o;
}
