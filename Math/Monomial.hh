/// \file "Monomial.hh" Monomial term for symbolic polynomial manipulation
/*
 * Monomial.hh, part of the MPMUtils package.
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

#ifndef MONOMIAL_HH
#define MONOMIAL_HH

#include <iostream>
using std::ostream;
#include <iomanip>
#include <exception>
#include <cassert>
#include <cmath>

/// general exception for polynomial problems
class PolynomialException: public std::exception {
    virtual const char* what() const throw() { return "Polynomial Problem!"; }
};

/// exception for attempts to use inconsistent units
class InconsistentMonomialException: public PolynomialException {
    virtual const char* what() const throw() { return "Incomparable monomial terms!"; }
};

/// Monomial (one term in a Polynomial), given by coefficient and vector of exponents
template<typename Vec, typename T>
class Monomial: public Vec {
public:
    typedef Vec exponents;  ///< exponents list
    typedef T value;        ///< evaluation value

    template<typename A>
    using array_contents_t = typename std::remove_reference<decltype(std::declval<A&>()[0])>::type;

    /// constructor
    Monomial(T v, Vec d = Vec()): Vec(d), coeff(v) { }

    /// check Monomial consistency
    bool consistentWith(const Monomial& u) const { return (Vec&)(*this) == (Vec&)u; }
    /// throw error if inconsistent
    void assertConsistent(const Monomial& u) const { if(!consistentWith(u)) throw(InconsistentMonomialException()); }

    /// convert to dimensionless quantity of given unit
    double in(const Monomial& u) const { assertConsistent(u); return coeff/u.coeff; }

    /// unary minus
    const Monomial operator-() const { return Monomial(-coeff, *this); }
    /// inverse Monomial
    const Monomial inverse() const { Monomial u(1./coeff, *this); for(auto& e: u) { e *= -1; } return u; }

    /// evaluate at given point
    template<typename coord>
    auto operator()(const coord& v) const -> array_contents_t<coord> {
        assert(v.size() == this->size());
        array_contents_t<coord> s = coeff;
        unsigned int i=0;
        for(auto e: *this) { if(e) s *= pow(v[i++],e); }
        return s;
    }

    /// inplace addition
    Monomial& operator+=(const Monomial& rhs) { assertConsistent(rhs); coeff += rhs.coeff; return *this; }
    /// inplace subtraction
    Monomial& operator-=(const Monomial& rhs) { assertConsistent(rhs); coeff -= rhs.coeff; return *this; }
    /// inplace multiplication
    Monomial& operator*=(const Monomial& rhs) { unsigned int i=0; for(auto& e: *this) { e += rhs[i++]; } coeff *= rhs.coeff; return *this; }
    /// inplace division
    Monomial& operator/=(const Monomial& rhs) { unsigned int i=0; for(auto& e: *this) { e -= rhs[i++]; } coeff /= rhs.coeff; return *this; }
    /// inplace multiplication
    Monomial& operator*=(T rhs) { coeff *= rhs; return *this; }
    /// inplace division
    Monomial& operator/=(T rhs) { coeff /= rhs; return *this; }

    /// addition operator
    const Monomial operator+(const Monomial& other) const { return Monomial(*this) += other; }
    /// subtraction operator
    const Monomial operator-(const Monomial& other) const { return Monomial(*this) -= other; }
    /// multiplication operator
    const Monomial operator*(const Monomial& other) const { return Monomial(*this) *= other; }
    /// division operator
    const Monomial operator/(const Monomial& other) const { return Monomial(*this) /= other; }
    /// multiplication operator
    const Monomial operator*(T other) const { return Monomial(*this) *= other; }
    /// division operator
    const Monomial operator/(T other) const { return Monomial(*this) /= other; }

    /// output representation in algebraic form
    ostream& algebraicForm(ostream& o) const {
        o << std::showpos << coeff << std::noshowpos;
        unsigned int i = 0;
        for(auto e: *this) {
            if(e) {
                o << vletters[i];
                if(e != 1) o << "^" << e;
            }
            i++;
        }
        return o;
    }

    T coeff;  ///< coefficient
    static constexpr const char* vletters = "xyztuvwabcdefghijklmnopqrs";  ///< letters for variable names
};

/// output representation
template<typename Vec, typename T>
ostream& operator<<(ostream& o, const Monomial<Vec,T>& u) { return u.algebraicForm(o); }

/*
 *    ostream& latexForm(ostream& o) const {
 *        o << std::showpos << coeff << std::noshowpos;
 *        for(P i=0; i<N; i++) {
 *            if((*this)[i] > 0) {
 *                if((*this)[i] == 1) {
 *                    o << vletters[i];
 *                } else {
 *                    o << vletters[i] << "^";
 *                    if((*this)[i] < 10)
 *                        o << (*this)[i] ;
 *                    else
 *                        o << "{" << (*this)[i] << "}";
 *                }
 *            }
 *        }
 *        return o;
 *    }
 *
 *    template<typename Vec, typename P>
 *    ostream& tableForm(ostream& o) const {
 *        o << std::setw(20) << std::setprecision(10) << coeff << "\t";
 *        for(P i=0; i<N; i++)
 *            o << " " << std::setw(0) << (*this)[i];
 *        return o;
 *    }
 */


#endif
