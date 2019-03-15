/// \file Eratosthenes.cc

#include "Eratosthenes.hh"
#include <algorithm>

PrimeSieve& theSieve() {
    static PrimeSieve PS;
    return PS;
}

PrimeSieve::PrimeSieve(size_t i0):
factors({{0}, {}}), factor_max(1) {
    while(factors.size()<i0)
        checkNext();
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

    // expand table as needed up to sqrt(i)
    while(i > factor_max) {
        auto d = checkNext();
        if(d && !(i%d)) { // d is a prime && factor of i?
            auto v = _factor(i/d);
            v.push_back(d);
            std::sort(v.begin(), v.end());
            addXF(i,v);
            return v;
        }
    }

    // search next prime factor
    size_t f = 0;
    for(auto p: primes) {
        if(!(i%p)) break;
        f++;
    }

    if(f == primes.size()) { // i is prime!
        factors_t v;
        v.push_back(i);
        addXF(i,v);
        return v;
    }

    auto d = primes[f]; // the divisor of i
    auto v = _factor(i/d);
    v.push_back(d);
    std::sort(v.begin(), v.end());
    addXF(i,v);
    return v;
}

PrimeSieve::int_t PrimeSieve::checkNext() {
    const int_t i = factors.size();
    factor_max += 2*factors.size()-1;

    // previously-identified factorization?
    auto it = xf.find(i);
    if(it != xf.end()) {
        bool isPrime = it->second.size() < 2;
        if(isPrime) primes.push_back(i);
        factors.push_back(it->second);
        xf.erase(it);
        return isPrime? i : 0;
    }

    int_t f = 0;
    for(auto p: primes) {
        if(!(i%p)) break;
        f++;
    }
    bool isPrime = f == primes.size();

    if(isPrime) {
        primes.push_back(i);
        factors.push_back({i});
    } else {
        auto d = primes[f]; // divisor of i
        factors.push_back(factors[i/d]);
        factors.back().push_back(d);
        std::sort(factors.back().begin(), factors.back().end());
    }

    return isPrime? i : 0;
}

PrimeSieve::int_t PrimeSieve::prod(const factors_t& f) {
    int_t i = 1;
    for(auto& p: f) i *= p;
    return i;
}
