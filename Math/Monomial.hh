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
#include <iomanip>
#include <exception>
#include <cassert>
#include <cmath>
#include <array>

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
    typedef T coeff_t;      ///< coefficient
    typedef Vec exponents;  ///< exponents list

    template<typename A>
    using array_contents_t = typename std::remove_reference<decltype(std::declval<A&>()[0])>::type;

    typedef array_contents_t<Vec> exp_t;    ///< exponent type

    /// constructor
    Monomial(coeff_t v, exponents d = exponents()): Vec(d), coeff(v) { }

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

    /// derivative of i^th variable
    const Monomial derivative(size_t i) const {
        auto e = (*this)[i];
        if(!e) return Monomial(0);
        auto m = (*this)*e;
        m[i]--;
        return m;
    }
    /// indefinite integral of i^th variable
    const Monomial integral(size_t i) const {
        auto e = (*this)[i];
        assert((int)e != -1);
        auto m = (*this)/(e+1.);
        m[i]++;
        return m;
    }
    /// replace i^th variable with constant
    const Monomial eval(size_t i, coeff_t c) const { auto m = (*this)*pow(c, (*this)[i]); m[i] = 0; return m; }
    /// definite integral of i^th variable
    const Monomial integral(size_t i, coeff_t c0, coeff_t c1) const {
        auto m = this->integral(i);
        m *= (pow(c1, m[i]) - pow(c0, m[i]));
        m[i] = 0;
        return m;
    }

    /// evaluate at given point
    template<typename coord>
    auto operator()(const coord& v) const -> array_contents_t<coord> {
        assert(v.size() == this->size());
        array_contents_t<coord> s = coeff;
        unsigned int i=0;
        for(auto e: *this) { if(e) s *= pow(v[i++],e); }
        return s;
    }

    /// equality check
    bool operator==(const Monomial& m) const { return coeff == m.coeff && (Vec&)*this == m; }

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

    /// polynomial order
    exp_t order() const { exp_t o = 0; for(auto e: *this) o += fabs(e); return o; }
    /// is this term even?
    bool isEven() const { for(auto e: *this) { if(e%2) return false; } return true; }
    /// is this term odd?
    bool isOdd() const { for(auto e: *this) { if(!(e%2)) return false; } return true; }

    /// output representation in algebraic form
    std::ostream& algebraicForm(std::ostream& o, bool LaTeX=false) const {
        if(fabs(coeff) == 1) {
            if(!this->order()) {
                o << std::showpos << coeff << std::noshowpos;
                return o;
            }
            o << (coeff > 0? "+":"-");
        } else o << std::showpos << coeff << std::noshowpos;

        unsigned int i = 0;
        for(auto e: *this) {
            if(e) {
                o << vletters[i];
                if(e != 1) o << (LaTeX? "^{" : "^") << e << (LaTeX? "}" : "");
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
std::ostream& operator<<(std::ostream& o, const Monomial<Vec,T>& u) { return u.algebraicForm(o); }

/// convenience typedef
template<long unsigned int N, typename T = double>
using Monomial_t = Monomial<std::array<unsigned int,N>, T>;

/// evaluate out variable
template<long unsigned int N, typename T>
Monomial_t<N-1, T> reduce(const Monomial_t<N,T>& m, int i, double c = 1) {
    Monomial_t<N-1, T> mm(c==1? m.coeff : m.coeff*pow(c,m[i]));
    size_t j = 0, k = 0;
    for(auto e: m) {
        if((int)j++ == i) continue;
        mm[k++] = e;
    }
    return mm;
}

/*

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
