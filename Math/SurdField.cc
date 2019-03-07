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

    if(!R) return SurdSum();

    PrimeRoot_t r;
    Rational::fmap_t ifact;

    if(!R.positive) r.insert(-1);

    for(auto& kv: R) {
        auto x = kv.first;
        if(x == 1) continue;

        //sqrt(abc/def) = sqrt(abcdef)/def
        auto k = kv.second;
        if(abs(k)%2) r.insert(x);

        if(k < -1) ifact[x] -= (1-k)/2;
        else if(k > 1) ifact[x] += k/2;
    }

    SurdSum SS;
    SS.emplace(r, Rational(ifact));
    return SS;
}

void SurdSum::invert() {
    std::cout << "Inverting " << *this << "\n";

    assert(size());

    auto t = rbegin();
    auto rr = t->first.square();
    auto k  = t->second;

    if(size() == 1) {
        // 1/(k*r) = r/(k*rr);
        t->second *= rr;
        t->second.invert();
    } else {
        // 1/(a+k*r) = (a-k*r)/(a^2 - k^2 r^2)
        auto S = *this;
        S.rbegin()->second = -S.rbegin()->second;
        erase(t->first);
        *this *= *this;
        *this += k*k*Rational(-rr);
        invert();
        *this *= S;
    }
}

SurdSum& SurdSum::operator*=(const SurdSum& R) {
    // special case for multiply-by-0
    if(!R) *this = R;
    if(!(*this)) return *this;

    SurdSum P;

    for(auto& kv0: R) {
        for(auto& kv1: *this) {

            auto ip = kv0.first.mul(kv1.first);

            auto c = kv0.second * kv1.second * Rational(ip.first);

            auto it = P.find(ip.second);
            if(it == P.end()) P.emplace(ip.second, c);
            else {
                it->second += c;
                if(!it->second) P.erase(it);
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
            if(!it->second) erase(it);
        }
    }
    return *this;
}

std::ostream& operator<<(std::ostream& o, const SurdSum& r) {
    if(!r) o << "0";

    for(auto& kv: r) {
        auto c =  kv.first.square();
        //if(kv.second != 1 || c==1)
        o << kv.second;
        if(c != 1) o << "√" << c;
        o << " ";
    }

    return o;
}
