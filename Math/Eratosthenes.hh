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

#include <vector>
using std::vector;
#include <map>
using std::map;
#include <mutex>

class PrimeSieve {
public:
    /// Constructor, with option to "prime the pump"
    PrimeSieve(size_t i0 = 10000);
    /// Integer type handled
    typedef unsigned int int_t;
    /// factorization
    typedef vector<int_t> factors_t;

    /// get factorization (thread-safe)
    factors_t factor(int_t i);
    /// get factorization (not thread-safe)
    factors_t _factor(int_t i);

    /// check factors of next item in table; return i if prime, else 0 (not thread-safe)
    int_t checkNext();

    /// factors product
    static int_t prod(const factors_t& f);
    /// get primes list
    const vector<int_t>& getPrimes() const { return primes; }
    /// get extra primes
    const map<int_t, factors_t>& getXf() const { return xf; }
    /// get maximum checked number
    int_t maxchecked() const { return factors.size()-1; }

protected:
    /// add cached factorization
    void addXF(int_t i, const factors_t& v);

    std::mutex sieveLock;       ///< lock on updating sieve
    vector<int_t> primes;       ///< primes list
    vector<factors_t> factors;  ///< factorization table
    int_t factor_max;           ///< current largest factorable number
    map<int_t, factors_t> xf;   ///< spot factors for larger numbers outside table range
    size_t max_xf = 100000;     ///< maximum number of extra factors to cache
};

/// global singleton access
PrimeSieve& theSieve();

#endif
