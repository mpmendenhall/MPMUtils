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
#include <set>
using std::set;
#include <vector>
using std::vector;
#include <cmath>

/// Algebraic polynomial of monomials
template<typename M>
class Polynomial: protected set<M> {
public:

    typedef typename M::value value;
    template<typename A>
    using array_contents_t = typename std::remove_reference<decltype(std::declval<A&>()[0])>::type;

    /// constructor for 0 polynomial
    Polynomial(value c = 0) { if(c) this->insert(M(c)); }
    /// constructor from a monomial term
    Polynomial(const M& m) { this->insert(m); }

    /// generate polynomial with all terms of order <= o in each variable
    //static Polynomial allTerms(unsigned int o);
    /// generate polynomial with all terms of total order <= o
    //static Polynomial lowerTriangleTerms(unsigned int o);

    /// return polynomial with only even terms
    //Polynomial even() const;

    /// evaluate at given point
    template<typename coord>
    auto operator()(const coord& v) const -> array_contents_t<coord> { array_contents_t<coord> s = 0; for(auto& t: *this) { s += t(v); } return s; }

    /// evaluate a polynomial change of variable
    template<typename pVec>
    Polynomial replace(const pVec& v) const {
        Polynomial P;
        for(auto t: *this) {
            Polynomial Q(t.coeff);
            size_t i = 0;
            for(auto e: t) {
                if(i >= v.size()) break;
                for(int j=0; j<e; j++) Q *= v[i];
                i++;
            }
            P += Q;
        }
        return P;
    }
    /// expand polynomial around a new origin
    template<typename cvec>
    const Polynomial recentered(const cvec& v) const {
        vector<Polynomial> vP;
        size_t i = 0;
        for(auto c: v) {
            M m(1);
            assert(i < m.size());
            m[i++] = 1;
            vP.push_back(Polynomial(m)-Polynomial(c));
        }
        return this->replace(v);
    }
    /// remove negligible terms
    Polynomial& prune(value c = 0) {
        Polynomial P;
        for(auto& t: *this) if(fabs(t.coeff) > c) P.insert(t);
        *this = P;
        return *this;
    }

    /// inplace addition
    Polynomial& operator+=(const Polynomial& rhs) {
        for(auto& t: rhs) {
            auto it = this->find(t);
            if(it == this->end()) this->insert(t);
            else {
                (M&)(*it) += t;
                if(!it->coeff) this->erase(it);
            }
        }
        return *this;
    }

    /// inplace subtraction
    Polynomial& operator-=(const Polynomial& rhs)  {
        for(auto& t: rhs) {
            auto it = this->find(t);
            if(it == this->end()) this->insert(t);
            else {
                (M&)(*it) -= t;
                if(!it->coeff) this->erase(it);
            }
        }
        return *this;
    }

    /// inplace multiplication by a polynomial
    Polynomial& operator*=(const Polynomial& rhs) { *this = (*this) * rhs; return *this; }
    /// inplace division by a monomial
    Polynomial& operator/=(const M& rhs) { for(auto& t: *this) { (M&)t /= rhs; } return *this; }
    /// inplace multiplication by a constant
    Polynomial& operator*=(value c) { for(auto& t: *this) { (M&)t *= c; } return *this; }
    /// inplace division by a constant
    Polynomial& operator/=(value c) { for(auto& t: *this) { (M&)t /= c; } return *this; }

    /// addition
    const Polynomial operator+(const Polynomial& rhs) const { auto p = *this; p += rhs; return p; }
    /// subtraction
    const Polynomial operator-(const Polynomial& rhs) const { auto p = *this; p -= rhs; return p; }
    /// multiplication
    const Polynomial operator*(const Polynomial& rhs) const { Polynomial P; for(auto& t: *this) { for(auto& t2: rhs) P += t*t2; }; return P; }
    /// multiplication by a scalar
    const Polynomial operator*(value c) const { auto p = *this; p *= c; return p; }
    /// division
    const Polynomial operator/(const M& rhs) const { auto p = *this; p *= rhs; return p; }
    /// division by a scalar
    const Polynomial operator/(value c) const { auto p = *this; p /= c; return p; }

    /// output in table form
    //ostream& tableForm(ostream& o) const;
    /// output in LaTeX form
    //ostream& latexForm(ostream& o) const;

    ostream& algebraicForm(ostream& o) const { for(auto& t: *this) { o << t; } if(!this->size()) o << "0"; return o; }
};

/// output representation
template<typename M>
ostream& operator<<(ostream& o, const Polynomial<M>& u) { return u.algebraicForm(o); }

/*


template<unsigned int N, typename T>
Polynomial<N,T> Polynomial<N,T>::allTerms(unsigned int o) {
    Vec<N,unsigned int> d = Vec<N,unsigned int>();
    Polynomial<N,T> t = Polynomial<N,T>(Monomial<N,T,unsigned int>(1,d));
    unsigned int p = 0;
    while(p<N) {
        if(d[p] < o) {
            d[p] += 1;
            t.terms[d] = 1.0;
            p = 0;
        } else {
            d[p] = 0;
            p++;
        }
    }
    return t;
}

template<unsigned int N, typename T>
Polynomial<N,T> Polynomial<N,T>::lowerTriangleTerms(unsigned int o) {
    Polynomial<N,T> lt = Polynomial<N,T>();
    Polynomial<N,T> t = Polynomial<N,T>::allTerms(o);
    for(auto const& kv: t.terms)
        if(kv.first.sum() <= o)
            lt.terms[kv.first] = 1.0;
    return lt;
}

template<unsigned int N, typename T>
Polynomial<N,T> Polynomial<N,T>::even() const {
    Polynomial<N,T> e = Polynomial<N,T>();
    for(auto const& kv: terms) {
        bool iseven = true;
        for(unsigned int i=0; i<N; i++) {
            if(kv.first[i].denominator() != 1 || (kv.first[i].numerator())%2 ) {
                iseven = false;
                break;
            }
        }
        if(iseven)
            e.terms[kv.first] += kv.second;
    }
    return e;
}

template<unsigned int N, typename T>
ostream& Polynomial<N,T>::latexForm(ostream& o) const {
    for(auto const& kv: terms) Monomial<N,T,unsigned int>(kv.second, kv.first).latexForm(o);
    return o;
}

template<unsigned int N, typename T>
ostream& Polynomial<N,T>::tableForm(ostream& o) const {
    for(auto const& kv: terms) {
        Monomial<N,T,unsigned int>(kv.second,kv.first).tableForm(o);
        o << "\n";
    }
    return o;
}
*/

#endif
