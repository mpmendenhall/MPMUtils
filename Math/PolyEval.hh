/// @file PolyEval.hh Vectorized evaluation of polynomials
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
#include "Abstract.hh"

/// Fast vectorized evaluation of polynomials at many points
template<typename T = double>
class PolyEval {
public:
    /// convenience typedef for n-dimensional evauation coordinate
    template<unsigned int n>
    using coord_t = array<T,n>;

    /// load x,y,z,... vectors
    template<class Cvec>
    void setX(const Cvec& v) {
        Xd.clear();
        npts = v.size();
        if(!v.size()) return;

        auto n = v[0].size();
        for(size_t i = 0; i < n; i++) {
            auto& x = Xd[i];
            x.Xs.resize(v.size());
            size_t j = 0;
            for(auto& c: v) x.Xs[j++] = c[i];
            x.Ps.setX(x.Xs);
        }
    }

    /// Evalualuate standard-form polymial term into vector
    template<class M>
    void evalMonomial(const M& m, vector<T>& v) {
        v.resize(0);
        v.resize(npts, 1);
        for(auto& kv: m) Xd.at(kv.first).Ps.mul(v, kv.second);
    }

    /// Add evaluated monomial to input vector; auto-resize if input vector empty
    template<class M>
    void addMonomial(const M& m, const T& coeff, vector<T>& v) {
        if(!v.size()) v.resize(npts);

        vector<T> vv;
        evalMonomial(m,vv);
        size_t i = 0;
        for(auto c: vv) v[i++] += coeff*c;
    }

    /// Add evaluated polynomial to input vector; auto-resize if input vector empty
    template<class P>
    void addPolynomial(const P& p, vector<T>& v) { for(auto& kv: p) addMonomial(kv.first.get(), kv.second, v); }

    /// Evaluate polynomial into vector
    template<class P>
    void evalPolynomial(const P& p, vector<T>& v) { v.clear(); addPolynomial(p,v); }


protected:
    size_t npts = 0;                    ///< number of evaluation points
    struct Xdata {
        vector<T> Xs;                   ///< one variable's values
        PowerSeriesEval<vector<T>> Ps;  ///< powers of the variable
    };
    map<int,Xdata> Xd;                  ///< cached data
};

#endif
