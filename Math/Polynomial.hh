/// \file "Polynomial.hh" Algebraic polynomial manipulation
/*
 * Polynomial.hh, part of the MPMUtils package.
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


#ifndef POLYNOMIAL_HH
#define POLYNOMIAL_HH

#include "Monomial.hh"
#include <vector>
#include <cmath>

/// Algebraic polynomial of monomials
template<typename M, typename R>
class Polynomial: public AbstractPolynomial<R,M> {
public:
    typedef M monomial_t;
    typedef R coeff_t;

    /// default empty polynomial constructor
    Polynomial() { }
    /// constructor from a monomial term
    Polynomial(const M& m, const coeff_t& c = 0) { (*this)[m] = c; }

    /// generate polynomial with all terms of order <= o in each variable
    static Polynomial allTerms(unsigned int o, coeff_t c = 0) {
        M m;
        Polynomial P(m, c);
        unsigned int i = 0;
        while(i < m.size()) {
            if((unsigned int)m[i] < o) {
                m[i]++;
                P[m] = c;
                i = 0;
            } else m[i++] = 0;
        }
        return P;
    }

    /// generate polynomial with all terms of total order <= o
    static Polynomial lowerTriangleTerms(unsigned int o, coeff_t c = 0) {
        auto P0 = allTerms(o,c);
        Polynomial P;
        for(auto& kv: P0)
            if((unsigned int)kv.first.order() <= o)
                P.insert(kv);
        return P;
    }

    /// evaluate at given point
    template<typename coord>
    auto operator()(const coord& v) const -> array_contents_t<coord> {
        array_contents_t<coord> s = 0;
        for(auto& kv: *this) s += kv.first(v) * kv.second;
        return s;
    }

    /// derivative of i^th variable
    const Polynomial derivative(size_t i) const {
        Polynomial P;
        for(auto& kv: *this) {
            auto m = kv.first;
            auto c = m.differentiate(i);
            if(!c) continue;
            P[m] = kv.second * c;
        }
        return P;
    }

    /// indefinite integral of i^th variable
    const Polynomial integral(size_t i) const {
        Polynomial P;
        for(auto& kv: *this) {
            auto m = kv.first;
            double c = m.integrate(i);
            P[m] = kv.second / c;
        }
        return P;
    }
    /// evaluate with i^th variable set to constant
    //const Polynomial eval(size_t i, coeff_t c) const { Polynomial P; for(auto& m: *this) P += m.eval(i,c); return P; }

    /*
    /// definite integral of i^th variable
    const Polynomial integral(size_t i, coeff_t c0, coeff_t c1) { Polynomial P; for(auto& m: *this) P += m.integral(i,c0,c1); return P; }

    /// evaluate a polynomial change of variable
    template<typename pVec>
    Polynomial replace(const pVec& v) const {
        Polynomial P;
        for(auto t: *this) {
            Polynomial Q(t.coeff);
            size_t i = 0;
            for(auto e: t) {
                if(i >= v.size()) break;
                for(int j=0; j<(int)e; j++) Q *= v[i];
                i++;
            }
            P += Q;
        }
        return P;
    }

    /// expand polynomial around a new origin
    template<typename cvec>
    const Polynomial recentered(const cvec& v) const {
        std::vector<Polynomial> vP;
        size_t i = 0;
        for(auto c: v) {
            M m(1);
            assert(i < m.size());
            m[i++] = 1;
            vP.push_back(Polynomial(m)-Polynomial(c));
        }
        return this->replace(v);
    }
*/
    /// remove negligible terms
    Polynomial& prune(coeff_t c = 0) {
        Polynomial P;
        for(auto& kv: *this) if(fabs(kv.second) > c) P.insert(kv);
        *this = P;
        return *this;
    }

    /// print representation
    std::ostream& algebraicForm(std::ostream& o, bool LaTeX=false) const {
        for(auto& kv: *this) {
            if(fabs(kv.second) == 1) {
                if(!kv.first.order()) {
                    o << std::showpos << double(kv.second) << std::noshowpos;
                    continue;
                }
                o << (kv.second > 0? "+":"-");
            } else o << std::showpos << double(kv.second) << std::noshowpos;

            kv.first.algebraicForm(o, LaTeX) << " ";
        }
        if(!this->size()) o << "0";
        return o;
    }
};

/// output representation
template<typename M, typename R>
std::ostream& operator<<(std::ostream& o, const Polynomial<M,R>& u) { return u.algebraicForm(o); }

/// convenience typedef
template<long unsigned int N, typename T = double>
using Polynomial_t = Polynomial<Monomial_t<N>,T>;

/// evaluate out variable
template<long unsigned int N, typename T = double>
Polynomial_t<N-1, T> reduce(const Polynomial_t<N,T>& p, int i, T c = 1) {
    Polynomial_t<N-1, T> pp;
    for(auto& m: p) pp += reduce(m,i,c);
    return pp;
}

#endif
