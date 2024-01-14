/// @file PowerSeriesEval.hh Vectorized power-series sum evaluation
/*
 * PowerSeriesEval.hh, part of the MPMUtils package.
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

#ifndef POWERSERIESEVAL_HH
#define POWERSERIESEVAL_HH

#include <vector>

/// Vectorized power-series sum evaluation
template<typename Vec>
class PowerSeriesEval {
public:
    /// return value, deduced from vector type
    typedef typename std::remove_reference<decltype(std::declval<Vec&>()[0])>::type T;

    /// set x^1 values
    void setX(const Vec& vX) { X = &vX; Xn.clear(); }

    /// add k*x^n to vector
    void add(Vec& v0, T k, size_t n) {
        if(!n) for(auto& s: v0) s += k;
        else {
            const auto& x = pow(n);
            size_t i = 0;
            for(auto& s: v0) s += k*x[i++];
        }
    }

    /// multiply vector by x^n
    void mul(Vec& v0, size_t n) {
        if(!n) return;
        const auto& x = pow(n);
        size_t i = 0;
        for(auto& s: v0) s *= x[i++];
    }

protected:
    /// get x^n vector
    const Vec& pow(size_t n) {
        assert(X);
        if(n==1) return *X;
        while(Xn.size()+2 <= n) {
            if(!Xn.size()) {
                Xn.push_back(*X);
                for(auto& i: Xn[0]) i *= i;
            } else {
                Xn.push_back(Xn.back());
                size_t i=0;
                for(auto& x: *X) Xn.back()[i++] *= x;
            }
        }
        return Xn[n-2];
    }

    const Vec* X = nullptr; ///< input X^1
    std::vector<Vec> Xn;    ///< powers of X
};

#endif
