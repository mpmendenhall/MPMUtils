/// \file Eratosthenes.hh Sieve of Eratosthenes primes/factoring utility
/*
 * Eratosthenes.hh, part of the MPMUtils package.
 * Copyright (c) 2007-2018 Michael P. Mendenhall
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */

#ifndef PRIMESIEVE_HH
#define PRIMESIEVE_HH

#include "DivisorCheck.hh"
#include <vector>
using std::vector;
#include <map>
using std::map;
#include <mutex>

/// Sieve of Eratosthenes primes/factoring utility
class PrimeSieve {
public:
    /// Constructor
    PrimeSieve(): factors({{0}, {}}) { }

    /// signed integer divisor math
    typedef long long int int_t;
    /// Unfactored integer type handled
    typedef typename std::make_unsigned<int_t>::type uint_t;

    /// factorization (sorted); empty vector for 1, {0} for 0
    typedef vector<uint_t> factors_t;

    /// get factorization (thread-safe)
    factors_t factor(uint_t i);
    /// get factorization (not thread-safe)
    factors_t _factor(uint_t i);

    /// check factors of next item in table; return i if prime, else 0 (not thread-safe)
    uint_t checkNext();

    /// factors product
    static uint_t prod(const factors_t& f);
    /// get primes list
    const vector<uint_t>& getPrimes() const { return primes; }
    /// get prime division checks
    const vector<DivisorCheck<int_t>> getPDivs() const { return pdivs; }
    /// get extra primes
    const map<uint_t, factors_t>& getXf() const { return xf; }
    /// get maximum checked number
    uint_t maxchecked() const { return factors.size()-1; }
    /// print status info to stdout
    void display() const;

protected:
    /// add cached factorization
    void addXF(uint_t i, const factors_t& v);

    std::mutex sieveLock;       ///< lock on updating sieve
    vector<uint_t> primes;      ///< primes list
    vector<DivisorCheck<int_t>> pdivs;  ///< divisor check for each prime
    vector<factors_t> factors;  ///< factorization table
    uint_t factor_max = 1;      ///< current largest factorable number
    map<uint_t, factors_t> xf;  ///< spot factors for larger numbers outside table range
    size_t max_xf = 1000000;    ///< maximum number of extra factors to cache
};

/// global singleton access
PrimeSieve& theSieve();

#endif
