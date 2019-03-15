/// \file SurdField.cc

#include "SurdField.hh"

std::pair<int, PrimeRoot_t> PrimeRoot_t::mul(const PrimeRoot_t& r) const {
    int i = 1;
    PrimeRoot_t p;
    for(auto x: r) {
        if(count(x)) i *= x;
        else p.insert(x);
    }
    for(auto x: *this) if(!r.count(x)) p.insert(x);

    return {i, p};
}

SurdSum SurdSum::sqrt(const Rational& R) {

    if(R == 0) return SurdSum();

    PrimeRoot_t r;
    Rational::fmap_t ifact; // perfect squares factored out

    // support imaginary numbers!
    if(!R.positive) r.insert(-1);

    for(auto& kv: R) {
        auto x = kv.first;
        if(x == 1) continue;

        //sqrt(abc/def) = sqrt(abcdef)/def
        auto k = kv.second;
        if(abs(k)%2) r.insert(x);

        if(k < 0) ifact[x] -= (1-k)/2;
        else ifact[x] += k/2;
    }

    SurdSum SS;
    SS.emplace(r, Rational(ifact));
    return SS;
}

pair<SurdSum,SurdSum> SurdSum::separateRoot(int i) const {
    if(i==1) return {*this, SurdSum(0)};

    pair<SurdSum,SurdSum> SS;
    for(auto& kv: *this) {
        auto ps = kv.first;
        if(ps.erase(i)) SS.first.emplace(ps, kv.second);
        else SS.second.emplace(ps, kv.second);
    }
    return SS;
}

void SurdSum::invert() {
    if(*this == 0) throw std::range_error("Refuse to calculate 1/0!");

    auto denom = *this; // remaining denominator
    *this = SurdSum(1); // inverse being constructed

    while(denom.size() > 1) {
        // choose factor r = sqrt(rr)
        auto rr = *(denom.rbegin()->first.rbegin());
        // split denom -> a + k*r
        auto ka = denom.separateRoot(rr);
        // S/(a+k*r) = S*(a-k*r)/(a^2 - k^2 r^2)
        *this *= ka.second - ka.first * sqrt(rr);
        denom = ka.second*ka.second - ka.first*ka.first*rr;
    }

    // S/(k*r) = r*S/(k*rr);
    auto rr = denom.begin()->first.square();
    auto k  = denom.begin()->second;
    k *= rr;
    k.invert();
    *this *= sqrt(Rational(rr)) * k;
}

SurdSum& SurdSum::operator*=(const SurdSum& R) {
    // special case for multiply-by-0
    if(R == 0) *this = R;
    if(*this == 0) return *this;

    SurdSum P;

    for(auto& kv0: R) {
        for(auto& kv1: *this) {

            auto ip = kv0.first.mul(kv1.first);

            auto c = kv0.second * kv1.second * Rational(ip.first);

            auto it = P.find(ip.second);
            if(it == P.end()) P.emplace(ip.second, c);
            else {
                it->second += c;
                if(it->second == 0) P.erase(it);
            }

        }
    }

    return (*this = P);
}

SurdSum& SurdSum::operator+=(const SurdSum& r) {
    for(auto& kv: r) {
        auto it = find(kv.first);
        if(it == end()) emplace(kv.first, kv.second);
        else {
            it->second += kv.second;
            if(it->second == 0) erase(it);
        }
    }
    return *this;
}

SurdSum& SurdSum::operator+=(const Rational& r) {
    if(r == 0) return *this;
    auto it = find(PrimeRoot_t());
    if(it == end()) emplace(PrimeRoot_t(), r);
    else {
        it->second += r;
        if(it->second == 0) erase(it);
    }
    return *this;
}

std::ostream& operator<<(std::ostream& o, const SurdSum& r) {
    o << "( ";
    if(r == 0) o << "0 ";

    for(auto& kv: r) {
        auto c =  kv.first.square();
        auto nd = kv.second.components();
        o << (kv.second.positive? "+" : "-");
        if(abs(nd.first) != 1 || c == 1) o << abs(nd.first);
        if(c != 1) o << "√" << c;
        if(nd.second != 1) o << "/" << nd.second;
        o << " ";
    }
    o << ")";
    return o;
}
