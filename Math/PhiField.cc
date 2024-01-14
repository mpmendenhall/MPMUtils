/// @file PhiField.cc

#include "PhiField.hh"

std::ostream& operator<<(std::ostream& o, const PhiField& r) {
    if(!r) return o << " 0";
    //if(r.a < 0 && 0 < r.b) return o << PhiField(0,r.b) << PhiField(r.a,0);

    if(r.a) o << (0 < r.a? "+" : "") << r.a;

    if(r.b) {
        auto nd = r.b.components();
        if(abs(nd.first) == 1) o << (r.b.positive? "+" : "-");
        else o << (r.b.positive? "+":"") << nd.first;
        o << "φ";
        if(nd.second > 1) o << "/" << nd.second;
    }

    return o;
}

bool PhiField::operator<(const PhiField& P) const {
    auto aa = a-P.a;
    auto bb = P.b-b;
    if(!bb) return aa.negdef();
    auto c = 2*aa/bb-1;
    return bb.positive? !c.positive || c*c < 5 : c.positive && 5 < c*c;
}

bool PhiField::operator<(const Rational& R) const {
    auto aa = a-R;
    if(!b) return aa.negdef();
    auto c = -2*aa/b-1;
    return b.positive? c.positive && 5 < c*c : !c.positive || c*c < 5;
}
