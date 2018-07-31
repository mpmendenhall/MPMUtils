/// \file Rational.cc

#include "Rational.hh"

Rational::Rational(int n, unsigned int d): positive(n >= 0) {
    if(!positive) n = -n;

    auto& PS = theSieve();
    if(d==1) *this = Rational(PS.factor(n));
    else {
        auto r1 = Rational(PS.factor(n));
        r1.positive = positive;
        auto r2 = Rational(PS.factor(d));
        r2.invert();
        *this = r1*r2;
    }
}

Rational::Rational(const PrimeSieve::factors_t& f): positive(true) {
    unsigned int i = 0;
    int n = 0;
    for(auto c: f) {
        if(c != i) {
            if(n) push_back({i,n});
            n=0;
        }
        ++n;
        i = c;
    }
    if(n) push_back({i,n});
}

Rational& Rational::operator*=(const Rational& R) {
    // special case for multiply-by-0
    if(!R) *this = R;
    if(!(*this)) return *this;

    (SGVec_t<>&)(*this) += R;
    if(!R.positive) positive = !positive;
    return *this;
}

pair<int,int> Rational::components() const {
    pair<int,int> n(1,1);
    for(auto& kv: *this) {
        auto e = kv.second;
        while(e > 0) { n.first *= kv.first; --e; }
        while(e < 0) { n.second *= kv.first; ++e; }
    }
    if(!positive) n.first = -n.first;
    return n;
}

Rational& Rational::operator+=(const Rational& r) {
    // special case for adding 0
    if(!r) return *this;
    if(!(*this)) { *this = r; return *this; }

    auto R = r;
    auto cf = relPrime(R);

    auto c1 = R.components();
    auto c2 = this->components();
    assert(c1.second == 1 && c2.second == 1);
    auto rf = Rational(c1.first + c2.first);

    (SGVec_t<>&)(*this) = cf;
    *this *= rf;
    return *this;
}

bool Rational::operator<(const Rational& R) const {
    auto a = components();
    auto b = R.components();
    return a.first * b.second < a.second * b.first;
}

std::ostream& operator<<(std::ostream& o, const Rational& r) {
    auto c = r.components();
    o << c.first;
    if(c.first && c.second != 1) o << "/" << c.second;
    return o;
}
