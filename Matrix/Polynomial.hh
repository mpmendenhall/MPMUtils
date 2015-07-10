/* 
 * Polynomial.hh, part of the MPMUtils package.
 * Copyright (c) 2007-2014 Michael P. Mendenhall
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

/// \file "Polynomial.hh" \brief Templatized polynomial manipulation
#ifndef POLYNOMIAL_HH
/// Make sure this header is only loaded once
#define POLYNOMIAL_HH

#include "Monomial.hh"
#include <map>
using std::map;

/// Templatized polynomial
template<unsigned int N, typename T>
class Polynomial {
public:
    /// constructor for 0 polynomial
    Polynomial() {}
    /// constructor from a monomial term
    Polynomial(Monomial<N,T,unsigned int> m) { terms[m.dimensions] = m.val; }
    /// destructor
    ~Polynomial() {}
    
    /// generate polynomial with all terms of order <= o in each variable
    static Polynomial<N,T> allTerms(unsigned int o);
    /// generate polynomial with all terms of total order <= o
    static Polynomial<N,T> lowerTriangleTerms(unsigned int o);
    
    /// return polynomial with only even terms
    Polynomial<N,T> even() const;
    
    /// evaluate at given point
    T operator()(const Vec<N,T>& v) const;
    /// evaluate a polynomial change of variable
    const Polynomial<N,T> operator()(const Vec< N,Polynomial<N,T> >& v) const;
    /// expand polynomial around a new origin
    const Polynomial<N,T> recentered(const Vec<N,T>& c) const;
    /// remove negligible terms
    Polynomial<N,T>& prune(T c = 0);
    
    /// inplace addition
    Polynomial<N,T>& operator+=(const Polynomial<N,T>& rhs);
    /// inplace subtraction
    Polynomial<N,T>& operator-=(const Polynomial<N,T>& rhs);
    /// inplace multiplication by a polynomial
    Polynomial<N,T>& operator*=(const Polynomial<N,T>& rhs);
    /// inplace multiplication by a constant
    Polynomial<N,T>& operator*=(T c);
    /// inplace division by a monomial
    Polynomial<N,T>& operator/=(const Monomial<N,T,unsigned int>& rhs);
    /// inplace division by a constant
    Polynomial<N,T>& operator/=(T c);
    
    /// addition
    const Polynomial<N,T> operator+(const Polynomial<N,T>& rhs) const;
    /// subtraction
    const Polynomial<N,T> operator-(const Polynomial<N,T>& rhs) const;
    /// multiplication
    const Polynomial<N,T> operator*(const Polynomial<N,T>& rhs) const;
    /// multiplication by a scalar
    const Polynomial<N,T> operator*(T c) const;
    /// division
    const Polynomial<N,T> operator/(const Monomial<N,T,unsigned int>& rhs) const;
    /// division by a scalar
    const Polynomial<N,T> operator/(T c) const;
    
    /// output representation, algebraic form
    ostream& algebraicForm(ostream& o) const;
    /// output in table form
    ostream& tableForm(ostream& o) const;
    /// output in LaTeX form
    ostream& latexForm(ostream& o) const;
    
    map<Vec<N,unsigned int>, T> terms;                     ///< terms of the polynomial
    typename map< Vec<N,unsigned int> , T >::iterator it;  ///< iterator for terms
};

template<unsigned int N, typename T>
T Polynomial<N,T>::operator()(const Vec<N,T>& v) const {
    T s = T();
    for(auto cit = terms.begin(); cit != terms.end(); cit++)
        s += Monomial<N,T,unsigned int>(cit->second,cit->first)(v);
    return s;
}

template<unsigned int N, typename T>
const Polynomial<N,T> Polynomial<N,T>::operator()(const Vec< N,Polynomial<N,T> >& v) const {
    Polynomial<N,T> P = Polynomial<N,T>();
    for(auto cit = terms.begin(); cit != terms.end(); cit++) {
        Polynomial<N,T> Q = Polynomial<N,T>(Monomial<N,T,unsigned int>(cit->second));
        for(unsigned int i = 0; i<N; i++)
            for(unsigned int j=0; j<cit->first[i]; j++)
                Q *= v[i];
            P += Q;
    }
    return P;
}

template<unsigned int N, typename T>
const Polynomial<N,T> Polynomial<N,T>::recentered(const Vec<N,T>& c) const {
    Vec< N,Polynomial<N,T> > v = Vec< N,Polynomial<N,T> >();
    for(unsigned int i=0; i<N; i++)
        v[i] = Polynomial<N,T>(Monomial<N,T,unsigned int>(1,Vec<N,unsigned int>::basis(i)))+Polynomial<N,T>(Monomial<N,T,unsigned int>(-c[i]));
    return (*this)(v);
}

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
    typename map< Vec<N,unsigned int> , T >::iterator it;
    for(it = t.terms.begin(); it != t.terms.end(); it++)
        if(it->first.sum() <= o)
            lt.terms[it->first] = 1.0;
        return lt;
}

template<unsigned int N, typename T>
Polynomial<N,T> Polynomial<N,T>::even() const {
    Polynomial<N,T> e = Polynomial<N,T>();
    for(auto cit = terms.begin(); cit != terms.end(); cit++) {
        bool iseven = true;
        for(unsigned int i=0; i<N; i++) {
            if(cit->first[i].denominator() != 1 || (cit->first[i].numerator())%2 ) {
                iseven = false;
                break;
            }
        }
        if(iseven)
            e.terms[cit->first] += cit->second;
    }
    return e;
}

template<unsigned int N, typename T>
Polynomial<N,T>& Polynomial<N,T>::operator+=(const Polynomial<N,T>& rhs) {
    for(auto cit = rhs.terms.begin(); cit != rhs.terms.end(); cit++)
        terms[cit->first] += cit->second;
    return *this;
}

template<unsigned int N, typename T>
Polynomial<N,T>& Polynomial<N,T>::operator-=(const Polynomial<N,T>& rhs) {
    for(auto cit = rhs.terms.begin(); cit != rhs.terms.end(); cit++)
        terms[cit->first] -= cit->second;
    return *this;
}

template<unsigned int N, typename T>
Polynomial<N,T>& Polynomial<N,T>::operator*=(const Polynomial<N,T>& rhs) {
    map<Vec<N,unsigned int>, T> newterms;
    for(it = terms.begin(); it != terms.end(); it++)
        for(auto cit = rhs.terms.begin(); cit != rhs.terms.end(); cit++)
            newterms[it->first + cit->first] += it->second*cit->second; 
        terms = newterms;
    return *this;
}

template<unsigned int N, typename T>
Polynomial<N,T>& Polynomial<N,T>::operator*=(T c) {
    for(it = terms.begin(); it != terms.end(); it++)
        it->second *= c;
    return *this;
}

template<unsigned int N, typename T>
Polynomial<N,T>& Polynomial<N,T>::operator/=(const Monomial<N,T,unsigned int>& rhs) {
    map<Vec<N,unsigned int>, T> newterms;
    for(it = terms.begin(); it != terms.end(); it++)
        newterms[it->first + rhs.dimensions] += it->second*rhs.val; 
    terms = newterms;
    return *this;
}

template<unsigned int N, typename T>
Polynomial<N,T>& Polynomial<N,T>::operator/=(T c) {
    for(it = terms.begin(); it != terms.end(); it++)
        it->second /= c;
    return *this;
}

template<unsigned int N, typename T>
Polynomial<N,T>& Polynomial<N,T>::prune(T c) {
    map<Vec<N,unsigned int>, T> newterms;
    for(it = terms.begin(); it != terms.end(); it++)
        if(it->second > c || it->second < -c)
            newterms[it->first] += it->second; 
        terms = newterms;
    return *this;
}

template<unsigned int N, typename T>
const Polynomial<N,T> Polynomial<N,T>::operator+(const Polynomial<N,T>& other) const {
    Polynomial<N,T> result = *this;
    result += other;
    return result;
}

template<unsigned int N, typename T>
const Polynomial<N,T> Polynomial<N,T>::operator-(const Polynomial<N,T>& other) const {
    Polynomial<N,T> result = *this;
    result -= other;
    return result;
}

template<unsigned int N, typename T>
const Polynomial<N,T> Polynomial<N,T>::operator*(const Polynomial<N,T>& other) const {
    Polynomial<N,T> result = *this;
    result *= other;
    return result;
}

template<unsigned int N, typename T>
const Polynomial<N,T> Polynomial<N,T>::operator*(T c) const {
    Polynomial<N,T> result = *this;
    result *= c;
    return result;
}


template<unsigned int N, typename T>
const Polynomial<N,T> Polynomial<N,T>::operator/(const Monomial<N,T,unsigned int>& other) const {
    Polynomial<N,T> result = *this;
    result /= other;
    return result;
}

template<unsigned int N, typename T>
const Polynomial<N,T> Polynomial<N,T>::operator/(T c) const {
    Polynomial<N,T> result = *this;
    result /= c;
    return result;
}

template<unsigned int N, typename T>
ostream& Polynomial<N,T>::algebraicForm(ostream& o) const {
    typename map< Vec<N,unsigned int> , T >::const_iterator it2;
    for(it2 = terms.begin(); it2 != terms.end(); it2++)
        Monomial<N,T,unsigned int>(it2->second, it2->first).algebraicForm(o);
    return o;
}

template<unsigned int N, typename T>
ostream& Polynomial<N,T>::latexForm(ostream& o) const {
    typename map< Vec<N,unsigned int> , T >::const_iterator it2;
    for(it2 = terms.begin(); it2 != terms.end(); it2++)
        Monomial<N,T,unsigned int>(it2->second, it2->first).latexForm(o);
    return o;
}

template<unsigned int N, typename T>
ostream& Polynomial<N,T>::tableForm(ostream& o) const {
    for(auto cit = terms.begin(); cit != terms.end(); cit++) {
        Monomial<N,T,unsigned int>(cit->second,cit->first).tableForm(o);
        o << "\n";
    }
    return o;
}

/// output representation
template<unsigned int N, typename T>
ostream& operator<<(ostream& o, const Polynomial<N,T>& p) {
    p.algebraicForm(o);
    return o;
}

#endif
