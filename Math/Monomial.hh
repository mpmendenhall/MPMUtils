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

#include "Abstract.hh"
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

/// Monomial function M, given by vector of exponents
template<typename Vec>
class Monomial: public Semigroup<Vec> {
public:
    typedef Semigroup<Vec> exponents;   ///< exponents list
    typedef array_contents_t<Vec> exp_t;///< exponent type

    /// constructor from exponents (defaults to unit)
    Monomial(const exponents& d = exponents()): exponents(d) { }

    /// derivative of i^th variable; return coefficient scaling
    exp_t differentiate(size_t i) { return (*this)[i]--; }

    /// indefinite integral of i^th variable; return inverse scaling
    exp_t integrate(size_t i) { return ++((*this)[i]); }

    /*
    /// replace i^th variable with constant
    const Monomial eval(size_t i, coeff_t c) const { auto m = (*this)*pow(c, (*this)[i]); m[i] = 0; return m; }
    /// definite integral of i^th variable
    const Monomial integral(size_t i, coeff_t c0, coeff_t c1) const {
        auto m = this->integral(i);
        m *= (pow(c1, m[i]) - pow(c0, m[i]));
        m[i] = 0;
        return m;
    }
    */

    /// polynomial order
    exp_t order() const { exp_t o = 0; for(auto e: *this) o += fabs(e); return o; }

    /// output representation in algebraic form
    std::ostream& algebraicForm(std::ostream& o, bool LaTeX=false) const {
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

    static constexpr const char* vletters = "xyztuvwabcdefghijklmnopqrs";  ///< letters for variable names
};

/// output representation
template<typename Vec>
std::ostream& operator<<(std::ostream& o, const Monomial<Vec>& u) { return u.algebraicForm(o); }

/// convenience typedef
template<long unsigned int N, typename T = unsigned int>
using Monomial_t = Monomial<std::array<T,N>>;

/*
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
*/

#endif
