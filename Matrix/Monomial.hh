/* 
 * Monomial.hh, part of the MPMUtils package.
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

/// \file "Monomial.hh" \brief Templatized monomial term for a polynomial
#ifndef MONOMIAL_HH
/// Make sure this header is only loaded once
#define MONOMIAL_HH

#include "Vec.hh"
#include <iostream>
#include <iomanip>
#include <exception>

/// general exception for polynomial problems
class PolynomialException: public std::exception {
    virtual const char* what() const throw() {
        return "Monomial Problem!";
    }
};

/// exception for attempts to use inconsistent units
class InconsistentMonomialException: public PolynomialException {
    virtual const char* what() const throw() {
        return "Incomparable monomial terms!";
    }
};

/// Monomial (one term in a Polynomial)
template<unsigned int N, typename T, typename P>
class Monomial {
public:
    /// constructor
    Monomial(T v, Vec<N,P> d): val(v), dimensions(d) { }
    /// constructor for dimensionless
    Monomial(T v): val(v), dimensions(Vec<N,P>()) { }
    /// destructor
    virtual ~Monomial() {}
    
    /// check Monomial consistency
    bool consistentWith(const Monomial<N,T,P>& u) const { return dimensions == u.dimensions; }
    /// throw error if inconsistent
    virtual void assertConsistent(const Monomial<N,T,P>& u) const { if(!consistentWith(u)) throw(InconsistentMonomialException()); }
    
    /// output representation in algebraic form
    ostream& algebraicForm(ostream& o) const;
    /// output representation in data table form
    ostream& tableForm(ostream& o) const;
    /// output representation in LaTeX form
    ostream& latexForm(ostream& o) const;
    
    /// convert to dimensionless quantity of given unit
    double in(const Monomial<N,T,P>& u) const { assertConsistent(u); return val/u.val; }
    
    /// unary minus
    const Monomial<N,T,P> operator-() const;
    /// inverse Monomial
    const Monomial<N,T,P> inverse() const;
    
    /// evaluate value at given point
    T operator()(const Vec<N,T>& v) const;
    
    /// inplace addition
    Monomial<N,T,P>& operator+=(const Monomial<N,T,P>& rhs);
    /// inplace subtraction
    Monomial<N,T,P>& operator-=(const Monomial<N,T,P>& rhs);
    /// inplace multiplication
    Monomial<N,T,P>& operator*=(const Monomial<N,T,P>& rhs);
    /// inplace division
    Monomial<N,T,P>& operator/=(const Monomial<N,T,P>& rhs);
    /// inplace multiplication
    Monomial<N,T,P>& operator*=(T rhs);
    /// inplace division
    Monomial<N,T,P>& operator/=(T rhs);
    /// addition operator
    const Monomial<N,T,P> operator+(const Monomial<N,T,P>& other) const;
    /// subtraction operator
    const Monomial<N,T,P> operator-(const Monomial<N,T,P>& other) const;
    /// multiplication operator
    const Monomial<N,T,P> operator*(const Monomial<N,T,P>& other) const;
    /// division operator
    const Monomial<N,T,P> operator/(const Monomial<N,T,P>& other) const;
    /// multiplication operator
    const Monomial<N,T,P> operator*(T other) const;
    /// division operator
    const Monomial<N,T,P> operator/(T other) const;
    
    T val;                              ///< dimensionless value
    Vec<N,P> dimensions;                ///< unit dimensions
    
    static const char* vletters;        ///< letters for variable names
    
    
};

template<unsigned int N, typename T, typename P>
const char* Monomial<N,T,P>::vletters = "xyztuvwabcdefghijklmnopqrs";

template<unsigned int N, typename T, typename P>
const Monomial<N,T,P> Monomial<N,T,P>::operator-() const {
    Monomial<N,T,P> u = *this;
    u.val = -val;
    return u;
}

template<unsigned int N, typename T, typename P>
const Monomial<N,T,P> Monomial<N,T,P>::inverse() const {
    Monomial<N,T,P> u = *this;
    u.val = 1./val;
    u.dimensions = -u.dimensions;
    return u;
}

template<unsigned int N, typename T, typename P>
T Monomial<N,T,P>::operator()(const Vec<N,T>& v) const {
    T s = val;
    for(unsigned int i=0; i<N; i++)
        s *= pow(v[i],dimensions[i]);
    return s;
}

template<unsigned int N, typename T, typename P>
Monomial<N,T,P>& Monomial<N,T,P>::operator+=(const Monomial<N,T,P>& rhs) {
    assertConsistent(rhs);
    val += rhs.val;
    return *this;
}

template<unsigned int N, typename T, typename P>
Monomial<N,T,P>& Monomial<N,T,P>::operator-=(const Monomial<N,T,P>& rhs) {
    assertConsistent(rhs);
    val -= rhs.val;
    return *this;
}

template<unsigned int N, typename T, typename P>
Monomial<N,T,P>& Monomial<N,T,P>::operator*=(const Monomial<N,T,P>& rhs) {
    dimensions += rhs.dimensions;
    val *= rhs.val;
    return *this;
}

template<unsigned int N, typename T, typename P>
Monomial<N,T,P>& Monomial<N,T,P>::operator/=(const Monomial<N,T,P>& rhs) {
    dimensions -= rhs.dimensions;
    val /= rhs.val;
    return *this;
}

template<unsigned int N, typename T, typename P>
Monomial<N,T,P>& Monomial<N,T,P>::operator*=(T rhs) {
    val *= rhs;
    return *this;
}

template<unsigned int N, typename T, typename P>
Monomial<N,T,P>& Monomial<N,T,P>::operator/=(T rhs) {
    val /= rhs;
    return *this;
}


template<unsigned int N, typename T, typename P>
const Monomial<N,T,P> Monomial<N,T,P>::operator+(const Monomial<N,T,P>& other) const {
    Monomial<N,T,P> result = *this;
    result += other;
    return result;
}

template<unsigned int N, typename T, typename P>
const Monomial<N,T,P> Monomial<N,T,P>::operator-(const Monomial<N,T,P>& other) const {
    Monomial<N,T,P> result = *this;
    result -= other;
    return result;
}

template<unsigned int N, typename T, typename P>
const Monomial<N,T,P> Monomial<N,T,P>::operator*(const Monomial<N,T,P>& other) const {
    Monomial<N,T,P> result = *this;
    result *= other;
    return result;
}

template<unsigned int N, typename T, typename P>
const Monomial<N,T,P> Monomial<N,T,P>::operator/(const Monomial<N,T,P>& other) const {
    Monomial<N,T,P> result = *this;
    result /= other;
    return result;
}

template<unsigned int N, typename T, typename P>
const Monomial<N,T,P> Monomial<N,T,P>::operator*(T other) const {
    Monomial<N,T,P> result = *this;
    result *= other;
    return result;
}

template<unsigned int N, typename T, typename P>
const Monomial<N,T,P> Monomial<N,T,P>::operator/(T other) const {
    Monomial<N,T,P> result = *this;
    result /= other;
    return result;
}


template<unsigned int N, typename T, typename P>
ostream& Monomial<N,T,P>::algebraicForm(ostream& o) const {
    o << std::showpos << val << std::noshowpos;
    for(P i=0; i<N; i++) {
        if(dimensions[i] > 0) {
            if(dimensions[i] == 1)
                o << vletters[i];
            else
                o << vletters[i] << "^" << dimensions[i];
        }
    }
    return o;
}

template<unsigned int N, typename T, typename P>
ostream& Monomial<N,T,P>::latexForm(ostream& o) const {
    o << std::showpos << val << std::noshowpos;
    for(P i=0; i<N; i++) {
        if(dimensions[i] > 0) {
            if(dimensions[i] == 1) {
                o << vletters[i];
            } else {
                o << vletters[i] << "^";
                if(dimensions[i] < 10)
                    o << dimensions[i] ;
                else
                    o << "{" << dimensions[i] << "}";
            }
        }
    }
    return o;
}


template<unsigned int N, typename T, typename P>
ostream& Monomial<N,T,P>::tableForm(ostream& o) const {
    o << std::setw(20) << std::setprecision(10) << val << "\t";
    for(P i=0; i<N; i++)
        o << " " << std::setw(0) << dimensions[i];
    return o;
}



/// output representation
template<unsigned int N, typename T, typename P>
ostream& operator<<(ostream& o, const Monomial<N,T,P>& u) {
    o << "[ " << u.val << " " << u.dimensions << " ]";
    return o;
}



#endif
