/// \file Eratosthenes.cc

#include "Eratosthenes.hh"
#include <algorithm>
#include <cassert>

PrimeSieve& theSieve() {
    static PrimeSieve PS;
    return PS;
}

PrimeSieve::factors_t PrimeSieve::factor(int_t i) {
    std::lock_guard<std::mutex> LG(sieveLock);
    return _factor(i);
}

void PrimeSieve::addXF(int_t i, const factors_t& v) {
    xf[i] = v;
    if(xf.size() > max_xf) xf.erase(std::prev(xf.end()));
}

PrimeSieve::factors_t PrimeSieve::_factor(int_t i) {
    // easy if previously checked
    if(i < factors.size()) return factors[i];
    auto it = xf.find(i);
    if(it != xf.end()) return it->second;

    req_max = std::max(i, req_max);

    for(auto p: primes) {
        if(!(i%p)) {
            auto v = _factor(i/p);
            v.push_back(p);
            std::sort(v.begin(), v.end());
            addXF(i,v);
            assert(i == prod(v));
            return v;
        }
    }

    // expand table as needed up to sqrt(i)
    while(i > factor_max) {
        auto d = checkNext();
        if(d && !(i%d)) { // d is a prime && factor of i?
            auto v = _factor(i/d);
            v.push_back(d);
            std::sort(v.begin(), v.end());
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

PrimeSieve::int_t PrimeSieve::checkNext() {
    const int_t i = factors.size();
    factor_max += 2*factors.size()-1;
    assert(factor_max == i*i);

    // previously-identified factorization?
    auto it = xf.find(i);
    if(it != xf.end()) {
        bool isPrime = it->second.size() < 2;
        if(isPrime) primes.push_back(i);
        factors.push_back(it->second);
        xf.erase(it);
        return isPrime? i : 0;
    }

    for(auto p: primes) {
        if(!(i%p)) {
            factors.push_back(factors[i/p]);
            factors.back().push_back(p);
            std::sort(factors.back().begin(), factors.back().end());
            return 0;
        }
    }

    primes.push_back(i);
    factors.push_back({i});
    return i;
}

PrimeSieve::int_t PrimeSieve::prod(const factors_t& f) {
    int_t i = 1;
    for(auto& p: f) {
        assert(i < std::numeric_limits<int_t>::max()/p);
        i *= p;
    }
    return i;
}

void PrimeSieve::display() const {
    printf("PrimeSieve with %zu + %zu factorizations using %zu primes\n",
            factors.size(), xf.size(), primes.size());
}
