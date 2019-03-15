/// \file Rational.cc

#include "Rational.hh"
#include <cstdlib> // for std::div
#include <cassert>

Rational::Rational(int n, int d) {
    if(!d) throw std::domain_error("Divide-by-0 is bad!");

    if(!n) {
        emplace_back(0,1);
        return;
    }

    auto& PS = theSieve();
    *this = Rational(PS.factor(abs(n)));
    positive = (n >= 0) == (d >= 0);
    if(abs(d) != 1) {
        Rational r2(abs(d),1);
        r2.invert();
        *this *= r2;
    }
}

Rational::Rational(const fmap_t& m, bool pos): positive(pos) {
    for(auto kv: m) {
        if(!kv.first) { *this = 0; return; }
        if(kv.first != 1 && kv.second) push_back(kv);
    }
}

Rational::Rational(const PrimeSieve::factors_t& f): positive(true) {
    // convert (a,a,b,b,b,c) -> a^2 b^3 c^1
    unsigned int i = 0;
    int n = 0;
    for(auto c: f) {
        assert(c);
        if(c != i) {
            assert(i != 1);
            if(n) push_back({i,n});
            n = 0;
        }
        ++n;
        i = c;
    }
    if(n) push_back({i,n});
}

Rational& Rational::invert() {
    if(isZero()) throw std::range_error("1/0 is bad!");
    if(isUnit()) return *this;
    for(auto& kv: *this) kv.second = -kv.second;
    return *this;
}

Rational& Rational::operator*=(const Rational& R) {
    // special case for multiply-by-0
    if(R.isZero()) *this = R;
    if(isZero()) return *this;

    positive = (positive == R.positive);

    // special case for +/-1
    if(R.isUnit()) return *this;
    if(isUnit()) {
        bool _pos = positive;
        *this = R;
        positive = _pos;
        return *this;
    }

    (SGVec_t<>&)(*this) += R;
    return *this;
}

bool Rational::operator==(int i) const {
    int j = 1;
    for(auto& kv: *this) {
        auto e = kv.second;
        if(e < 0) return false;
        while(e-- > 0) j *= kv.first;
    }
    return i == int(positive? j : -j);
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
    if(r.isZero()) return *this;
    if(isZero()) return *this = r;

    auto R = r;
    auto cf = relPrime(R); // common factors as SGVec_t of (factor,power); integer remainder in this, R

    auto c1 = R.components();
    auto c2 = this->components();
    assert(c1.second == 1 && c2.second == 1);
    int rf = c1.first + c2.first;

    (SGVec_t<>&)(*this) = cf;
    positive = true;
    return *this *= rf;
}

const Rational Rational::pow(int i) const {
    if(isZero()) {
        if(!i) throw std::range_error("0^0 is bad!");
        return 0;
    }
    if(!i) return 1;

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
