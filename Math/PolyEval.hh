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

#include "PowerSeriesEval.hh"
#include "Polynomial.hh"
using std::vector;

template<typename T = double>
class PolyEval {
public:

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

    /// Add evaluated monomial to input vector
    template<class M>
    void addMonomial(const M& m, vector<T>& v) {
        if(!Xs.size() || !v.size() || !m.coeff) return;
        assert(m.size() <= Xs.size() && v.size() <= Xs[0].size());

        vector<T> vv(v.size(), m.coeff);
        size_t i = 0;
        for(auto e: m) {
            if(!e) { i++;; continue; }
            while(Ps.size() <= i) {
                Ps.push_back({});
                Ps.back().setX(Xs[Ps.size()-1]);
            }
            vector<T> vvv(v.size());
            Ps[i].add(vvv, 1, e);
            size_t j = 0;
            for(auto& c: vv) c *= vvv[j++];
            i++;
        }

        i = 0;
        for(auto c: vv) v[i++] += c;
    }

    /// Add evaluated polynomial to input vector
    template<class P>
    void addPolynomial(const P& p, vector<T>& v) { for(auto& m: p) addMonomial(m,v); }

    /// Comparison simple algorithm
    template<class P, class Cvec>
    void addSimple(const P& p, vector<T>& v, const Cvec& c) const {
        assert(v.size() <= c.size());
        size_t i = 0;
        for(auto& y: v) y += p(c[i++]);
    }

protected:
    vector<vector<T>> Xs;                     ///< loaded coordinate vectors
    vector<PowerSeriesEval<vector<T>>> Ps;    /// Powers by variable
};
