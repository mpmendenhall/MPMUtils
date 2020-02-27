/// \file Rational.cc

#include "Rational.hh"
#include <cassert>
#include <limits>
#include <cmath>

const auto int_t_max = std::numeric_limits<Rational::int_t>::max();

Rational::Rational(int_t n) {
    if(!n) {
        emplace_back(0,1);
        return;
    }

    auto& PS = theSieve();
    *this = Rational(PS.factor(std::abs(n)));
    positive = n >= 0;
}


Rational::Rational(int_t n, int_t d) {
    if(!d) throw std::domain_error("Divide-by-0 is bad!");

    if(!n) {
        emplace_back(0,1);
        return;
    }

    auto& PS = theSieve();
    *this = Rational(PS.factor(std::abs(n)));
    positive = (n >= 0) == (d >= 0);
    if(std::abs(d) != 1) {
        Rational r2(std::abs(d),1);
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
    for(auto& kv: *this) kv.second = -kv.second;
    return *this;
}

Rational& Rational::operator*=(const Rational& R) {
    // special case for multiply-by-0
    if(R.isZero()) *this = R;
    if(isZero()) return *this;

    positive = (positive == R.positive);
    (super&)(*this) += R;
    return *this;
}

bool Rational::operator==(int_t i) const {
    if(isZero()) return i == 0;
    int_t j = 1;
    for(auto& kv: *this) {
        auto e = kv.second;
        if(e < 0) return false;
        while(e-- > 0) {
#ifndef NDEBUG
            if(!(j < int_t_max/kv.first)) throw std::range_error("Integer comparison overflow");
#endif
            j *= kv.first;
        }
    }
    return i == (positive? j : -j);
}

pair<Rational::int_t, Rational::int_t> Rational::components() const {
    if(isZero()) return {0,1};

    pair<int_t, int_t> n(1,1);
    for(auto& kv: *this) {
        auto e = kv.second;
        while(e > 0) {
#ifndef NDEBUG
            if(!(n.first < int_t_max/kv.first)) throw std::range_error("Numerator overflow");
#endif
            n.first *= kv.first;
            --e;
        }
        while(e < 0) {
#ifndef NDEBUG
            if(!(n.second < int_t_max/kv.first)) throw std::range_error("Denominator overflow");
#endif
            n.second *= kv.first;
            ++e;
        }
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

#ifndef NDEBUG
    if((c1.first > 0) == (c2.first >0) && std::abs(c1.first) > int_t_max - std::abs(c2.first)) throw std::range_error("Sum overflow!");
#endif
    c1.first += c2.first;

    (super&)(*this) = cf;
    positive = true;
    return *this *= c1.first;
}

const Rational Rational::pow(int i) const {
    if(isZero()) {
        if(!i) throw std::range_error("0^0 is bad!");
        return *this;
    }
    if(!i) return 1;

    Rational R = *this;
    for(auto& kv: R) kv.second *= i;
    if(!(i%2)) R.positive = true;
    return R;
}

bool Rational::operator<(const Rational& R) const {
    if(R.isZero()) return !positive;
    auto u = ((*this)/R).components();
    return R.positive? u.first < u.second : u.first > u.second;
}

std::ostream& operator<<(std::ostream& o, const Rational& r) {
    auto c = r.components();
    o << c.first;
    if(c.first && c.second != 1) o << "/" << c.second;
    return o;
}
