/// \file PolyEval.hh Vectorized evaluation of polynomials
/*
 * PolyEval.hh, part of the MPMUtils package.
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

#ifndef POLYEVAL_HH
#define POLYEVAL_HH

#include "PowerSeriesEval.hh"
#include "Polynomial.hh"
using std::vector;

/// Fast vectorized evaluation of polynomials at many points
template<typename T = double>
class PolyEval {
public:
    /// convenience typedef for n-dimensional evauation coordinate
    template<unsigned int n>
    using coord_t = std::array<T,n>;

    /// load x,y,z,... vectors
    template<class Cvec>
    void setX(const Cvec& v) {
        Xs.clear();
        Ps.clear();
        if(!v.size()) return;

        auto n = v[0].size();
        Xs.resize(n);
        for(size_t i = 0; i < n; i++) {
            Xs[i].resize(v.size());
            size_t j = 0;
            for(auto& c: v) Xs[i][j++] = c[i];
        }
    }

    /// Evalualuate monomial into vector
    template<class M>
    void evalMonomial(const M& m, vector<T>& v) {
        v.resize(0);
        assert(m.size() <= Xs.size());
        if(!Xs.size()) return;
        v.resize(Xs[0].size(), m.coeff);
        size_t i = 0;
        for(auto e: m) {
            while(Ps.size() <= i) {
                Ps.push_back({});
                Ps.back().setX(Xs[Ps.size()-1]);
            }
            Ps[i].mul(v, e);
            i++;
        }
    }

    /// Add evaluated monomial to input vector; auto-resize if input vector empty
    template<class M>
    void addMonomial(const M& m, vector<T>& v) {
        if(!v.size() && Xs.size()) v.resize(Xs[0].size());
        if(!Xs.size() || !m.coeff) return;

        vector<T> vv(v.size(), m.coeff);
        evalMonomial(m,vv);
        size_t i = 0;
        for(auto c: vv) v[i++] += c;
    }

    /// Add evaluated polynomial to input vector; auto-resize if input vector empty
    template<class P>
    void addPolynomial(const P& p, vector<T>& v) { for(auto& m: p) addMonomial(m,v); }

protected:
    vector<vector<T>> Xs;                     ///< loaded coordinate vectors
    vector<PowerSeriesEval<vector<T>>> Ps;    /// Powers by variable
};

#endif
