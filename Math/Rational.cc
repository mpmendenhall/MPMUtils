/// \file Rational.cc

#include "Rational.hh"

Rational::Rational(int n, int d) {
    assert(d);
    if(!n) {
        emplace_back(0,1);
        return;
    }

    auto& PS = theSieve();
    *this = Rational(PS.factor(abs(n)));
    positive = (n >= 0) == (d >= 0);
    if(d != 1) {
        Rational r2(abs(d),1);
        r2.invert();
        *this *= r2;
    }
}

Rational::Rational(const fmap_t& m, bool pos): positive(pos) {
    for(auto kv: m) if(kv.second) push_back(kv);
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
        assert(kv.first >= 0);
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
    auto cf = relPrime(R); // common factors as SGVec_t of (factor,power); integer remainder in this, R

    auto c1 = R.components();
    auto c2 = this->components();
    assert(c1.second == 1 && c2.second == 1);
    int rf = c1.first + c2.first;

    (SGVec_t<>&)(*this) = cf;
    positive = true;
    *this *= rf;
    return *this;
}

const Rational Rational::pow(int i) const {
    if(!i) { assert(!this); return Rational(1); }
    Rational R = *this;
    for(auto& kv: R) kv.second *= i;
    if(!(i%2)) R.positive = true;
    return R;
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
