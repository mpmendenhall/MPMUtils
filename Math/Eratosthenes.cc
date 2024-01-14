/// @file Eratosthenes.cc

#include "Eratosthenes.hh"
#include <algorithm>
#include <cassert>

PrimeSieve& theSieve() {
    static PrimeSieve PS;
    return PS;
}

PrimeSieve::factors_t PrimeSieve::factor(uint_t i) {
    std::lock_guard<std::mutex> LG(sieveLock);
    return _factor(i);
}

void PrimeSieve::addXF(uint_t i, const factors_t& v) {
    xf[i] = v;
    if(xf.size() > max_xf) xf.erase(std::prev(xf.end()));
}

PrimeSieve::factors_t PrimeSieve::_factor(uint_t i) {
    // easy if previously checked
    if(i < factors.size()) return factors[i];
    auto it = xf.find(i);
    if(it != xf.end()) return it->second;

    size_t pn = 0;
    for(auto p: primes) {
        if(pdivs[pn++].divides(i)) {
            assert(!(i%p));
            auto v = _factor(i/p);
            v.insert(std::lower_bound(v.begin(),v.end(),p),p);
            addXF(i,v);
            assert(i == prod(v));
            return v;
        } else assert(i%p);
    }

    // expand table as needed up to sqrt(i)
    while(i > factor_max) {
        auto d = checkNext();
        if(d && pdivs.back().divides(i)) { // d is a prime && factor of i?
            assert(!(i%d));
            auto v = _factor(i/d);
            v.insert(std::lower_bound(v.begin(),v.end(),d),d);
            addXF(i,v);
            assert(i == prod(v));
            return v;
        }
    }

    // i must be prime if no prime factors <= sqrt(i)
    factors_t v = {i};
    addXF(i,v);
    return v;
}

PrimeSieve::uint_t PrimeSieve::checkNext() {
    const uint_t i = factors.size();
    factor_max += 2*factors.size()-1;
    assert(factor_max == i*i);

    // previously-identified factorization?
    auto it = xf.find(i);
    if(it != xf.end()) {
        bool isPrime = it->second.size() < 2;
        if(isPrime) {
            primes.push_back(i);
            pdivs.emplace_back(i);
        }
        factors.push_back(it->second);
        xf.erase(it);
        return isPrime? i : 0;
    }

    size_t pn = 0;
    for(auto p: primes) {
        if(pdivs[pn++].divides(i)) {
            assert(!(i%p));
            factors.push_back(factors[i/p]);
            auto& v = factors.back();
            v.insert(std::lower_bound(v.begin(), v.end(), p), p);
            return 0;
        } else assert(i%p);
    }

    primes.push_back(i);
    pdivs.emplace_back(i);
    factors.push_back({i});
    return i;
}

PrimeSieve::uint_t PrimeSieve::prod(const factors_t& f) {
    uint_t i = 1;
    for(auto& p: f) {
        assert(!p || i < std::numeric_limits<uint_t>::max()/p);
        i *= p;
    }
    return i;
}

void PrimeSieve::display() const {
    printf("PrimeSieve with %zu + %zu factorizations using %zu primes\n",
            factors.size(), xf.size(), primes.size());
}
