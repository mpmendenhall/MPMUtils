/// \file "Abstract.hh" Top-level abstract mathematical structures
/*
 * Abstract.hh, part of the MPMUtils package.
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

#ifndef ABSTRACT_HH
#define ABSTRACT_HH

/////////////////////////////////////////////////////////////////////////////////////////
// Semigroup: set, operator +: a,b in S => a+b in s
//
// monoid: Semigroup with identity element 1 in S, 1*a = a*1 = a for all a in S
//
// group: monoid with inverses; for all a in S, exists a^-1 such that a a^-1 = 1 = a^-1 a
//
// ring: set, operator *, operator +
//      Abelian group under + => associative, commutative, has identity 0 and inverse -a
//      monoid under * (has identity 1); * is distributive over +, not necessarily commutative
//
// field: ring plus commutative over +, * inverses except 0
//
// vector/linear space: module over a field

// Left R-Module M:
// Abelian group (M,+)
// R is a Ring, operator * and ~
// operator x: r x m -> M
// r x (m+n) = r x m + r x n
// (r~s) x m = r x m + s x m
// (r*s) x m = r x (s x m)
// 1_R x m = m
// Right R-Module: similar with m x r -> M

#include <map>
#include <array>
#include <vector>
#include <iostream>
#include <cassert>
#include <cmath>
#include <algorithm>

using std::vector;
using std::map;
using std::array;
using std::pair;

/// Get type for contents of fixed array, std::array, std::vector
template<typename T>
using array_contents_t = typename std::remove_reference<decltype(std::declval<T&>()[0])>::type;

/// Get type for map key
template<typename M>
using map_key_t = typename std::remove_reference<decltype(std::declval<M&>().begin()->first)>::type;

/// Get type for map value
template<typename M>
using map_value_t = typename std::remove_reference<decltype(std::declval<M&>().begin()->second)>::type;

////////////////////////////////////////////////
////////////////////////////////////////////////

// Semigroup expectations:
// operators +=, +
// "standard form" = { {g1,k1}, {g2,k2}, ... }, generator ID's g to the k^th power
// typedef <standard form {{gi,ki},...}> elem_t;
// get standard-form element representation
// elem_t get() const { return <my element>; }

/*
/// output representation for standard-form Semigroup
template<class G>
std::ostream& operator<<(std::ostream& o, const Semigroup<G>& s) {
    auto e = s.get();
    o << "( ";
    for(auto& p: e) {
        if(p.second) {
            if(p.second != 1) o << p.second << "x";
            o << "[" << p.first << "]";
        } else o << "1";
        o << " ";
    }
    o << ")";
    return o;
}
*/

////////////////////////////////////////////////

/// Semigroup wrapper for using T::+
template<typename T>
class SemigroupPlus {
public:
    /// numbering type for generator multiplicity
    typedef T num_t;
    /// generator enumeration index --- only one generator!
    typedef int gen_t;
    /// Semigroup element standard form
    typedef vector<pair<gen_t,num_t>> elem_t;
    /// array size
    static constexpr auto N = 1;

    /// Constructor
    SemigroupPlus(const T& X = {}): x(X) { }

    T x;    ///< wrapped value

    /// get standard-form element representation
    elem_t get() const { return {{0, x}}; }

    /// comparison
    bool operator<(const SemigroupPlus& rhs) const { return x < rhs.x; }
    /// inplace addition
    SemigroupPlus& operator+=(const SemigroupPlus& rhs) { x += rhs.x; return *this; }
    /// out-of-place addition
    const SemigroupPlus operator+(const SemigroupPlus& rhs) const { return x + rhs.x; }
};

//////////////////////////////////////////////////
/// Semigroup operating element-wise on array type
template<typename A>
class ArraySemigroup: public A {
public:
    /// exponent type from array contents
    typedef array_contents_t<A> num_t;
    /// generator enumeration index
    typedef unsigned int gen_t;
    /// array size
    static constexpr auto N = std::tuple_size<A>::value;
    /// standard-form element type
    typedef vector<pair<gen_t,num_t>> elem_t;

    /// inherit Array constructors
    using A::A;
    /// Constructor from array
    ArraySemigroup(const A& a): A(a) { }
    /// Constructor for single-variable x_i^n
    ArraySemigroup(gen_t i, num_t n): A() { assert(i < this->size()); (*this)[i] = n; }

    /// get standard-form element representation
    elem_t get() const {
        elem_t g;
        gen_t i = 0;
        for(auto e: *this) {
            if(e) g.emplace_back(i,e);
            i++;
        }
        return g;
    }

    /// inplace addition
    ArraySemigroup& operator+=(const ArraySemigroup& rhs) {
        size_t i=0;
        for(auto& e: *this) e += rhs[i++];
        return *this;
    }
    /// out-of-place addition
    const ArraySemigroup operator+(const ArraySemigroup& rhs) const { auto s = *this; return s += rhs; }

    /// an appropriate coordinate type
    template<typename T>
    using coord_t = array<T,N>;

    /// evaluate at given point, treating as exponentiation
    template<typename coord>
    auto operator()(const coord& v) const -> array_contents_t<coord> {
        assert(v.size() == this->size());
        array_contents_t<coord> s = 1;
        unsigned int i = 0;
        for(auto e: *this) {
            while(e--) s *= v[i];
            i++;
        }
        return s;
    }
};

/// convenience typedef for N-dimensional SGArray of T (default uint)
template<long unsigned int N, typename T = unsigned int>
using SGArray_t = ArraySemigroup<array<T,N>>;

/////////////////////////////////////////

/// Specialization for element-wise Semigroup on tagged pairs (internally stored as sorted vector<pair<tag,x>>)
template<typename V>
class SVSemigroup: protected V {
public:
    /// base map object
    typedef V elem_t;
    /// one factor in element
    typedef array_contents_t<V> factor_t;
    /// first item is generator
    typedef typename std::remove_reference<decltype(std::declval<factor_t>().first)>::type gen_t;
    /// second item is exponent
    typedef typename std::remove_reference<decltype(std::declval<factor_t>().second)>::type num_t;

    /// comparison for sorting
    bool operator<(const SVSemigroup& rhs) const { return (V&)(*this) < (V&)(rhs); }

    /// default constructor
    SVSemigroup() { }
    /// Costructor for single-variable x_i^n
    SVSemigroup(gen_t i, num_t n) { if(n) this->push_back({i,n}); }

    /// get standard-form element representation
    elem_t get() const { return elem_t(*this); }

    /// inplace addition
    SVSemigroup& operator+=(const SVSemigroup& rhs) {
        elem_t vnew;
        auto it0 = this->begin();
        auto it1 = rhs.begin();
        while(it0 != this->end() && it1 != rhs.end()) {
            if(it0->first < it1->first) vnew.push_back(*(it0++));
            else if(it0->first > it1->first) vnew.push_back(*(it1++));
            else {
                auto p = *it0;
                p.second += it1->second;
                if(p.second) vnew.push_back(p);
                it0++; it1++;
            }
        }
        vnew.insert(vnew.end(), it0, this->end());
        vnew.insert(vnew.end(), it1, rhs.end());
        (V&)(*this) = vnew;
        return *this;
    }
    /// addition
    const SVSemigroup operator+(const SVSemigroup& rhs) const { auto s = *this; return s += rhs; }

    /// split negative-exponent generators into separate V
    SVSemigroup splitNegative() {
        auto it = std::stable_partition(this->begin, this->end(), [](factor_t f){return f.second > 0;});
        SVSemigroup SG;
        SG.assign(it, this->end());
        this->erase(it, this->end());
        return SG;
    }

    /// make this and other relatively prime (and positive), returning common factors
    SVSemigroup relPrime(SVSemigroup& S) {
        SVSemigroup P, ip0, ip1;
        auto it0 = this->begin();
        auto it1 = S.begin();

        while(it0 != this->end() && it1 != S.end()) {
            if(it0->first < it1->first) {
                if(it0->second < 0) {
                    P.push_back(*it0);
                    ip1.push_back({it0->first, -it0->second});
                    it0->second = 0;
                }
                ++it0;
            } else if(it0->first > it1->first) {
                if(it1->second < 0) {
                    P.push_back(*it1);
                    ip0.push_back({it1->first, -it1->second});
                    it1->second = 0;
                }
                ++it1;
            } else {
                num_t& a = it0->second;
                num_t& b = it1->second;
                num_t c = std::min(a,b);
                a -= c;
                b -= c;
                P.push_back({it0->first, c});
                it0++; it1++;
            }
        }

        while(it0 != this->end()) {
            if(it0->second < 0) {
                P.push_back(*it0);
                ip1.push_back({it0->first, -it0->second});
                it0->second = 0;
            }
            ++it0;
        }

        while(it1 != S.end()) {
            if(it1->second < 0) {
                P.push_back(*it1);
                ip0.push_back({it1->first, -it1->second});
                it1->second = 0;
            }
            ++it1;
        }

        *this += ip0;
        S += ip1;
        return P;
    }
};

/// convenience typedef for SVSemigroup
template<typename K = unsigned int, typename V = int>
using SGVec_t = SVSemigroup<vector<pair<K,V>>>;

/////////////////////////////////////////

/// Specialization for map-type key:value Semigroups, element-wise + on map values
template<typename M>
class MapSemigroup: public M {
public:
    /// base map object
    typedef M map_t;
    /// map key is generator
    typedef map_key_t<M> gen_t;
    /// map value is exponent
    typedef map_value_t<M> num_t;
    /// Semigroup element standard form
    typedef vector<pair<gen_t,num_t>> elem_t;

    /// default constructor
    MapSemigroup() { }
    /// constructor from map
    MapSemigroup(const M& m): M(m) { }
    /// Costructor for single-variable x_i^n
    MapSemigroup(gen_t i, num_t n) { (*this)[i] = n; }

    /// get standard-form element representation
    elem_t get() const {
        elem_t g;
        for(auto& kv: *this) g.emplace_back(kv);
        return g;
    }

    /// inplace addition
    MapSemigroup& operator+=(const MapSemigroup& rhs) {
        for(auto& kv: rhs) {
            auto it = this->find(kv.first);
            if(it == this->end()) {
                if(kv.second) this->insert(kv);
            } else {
                it->second += kv.second;
                if(it->second) this->erase(it);
            }
        }
        return *this;
    }
    /// addition
    const MapSemigroup operator+(const MapSemigroup& rhs) const { auto s = *this; return s += rhs; }
};

/// convenience typedef for SGMap of T (default uint)
template<typename K = unsigned int, typename V = unsigned int>
using SGMap_t = MapSemigroup<map<K,V>>;

////////////////////////////////////////////////
////////////////////////////////////////////////

/// Formal abstract polynomial, with +/- and * operations
// R is a ring with operators +,*, inverse -, default constructor '0', for the coefficients
// S is a Semigroup with operator + (monomial product) for monomial symbols (e.g. x, xy, x^2z ...)
// Using SGMap_t to allow easy term lookup/insertion
template<typename R, typename S, typename _X = void>
class AbstractPolynomial: public SGMap_t<S,R> {
public:
    /// Convenience typedef for base class
    typedef SGMap_t<S,R> SG;
    /// coefficients ring member type
    typedef R coeff_t;
    /// monomials Semigroup member type
    typedef S monomial_t;

    /// inherit SGMap_t constructors
    using SG::MapSemigroup;
    /// default constructor
    AbstractPolynomial() { }
    /// Costructor for constant
    AbstractPolynomial(const coeff_t& c) { (*this)[monomial_t{}] = c; }
    /// Costructor for single-variable x_i
    AbstractPolynomial(const coeff_t& c, typename monomial_t::gen_t i) { (*this)[monomial_t(i,1)] = c; }
    template<class U>
    AbstractPolynomial(const U& c) { (*this)[monomial_t{}] = R(c); }

    //////////////////////////////////////
    // core "required" polyomial functions

    /// inplace addition
    AbstractPolynomial& operator+=(const AbstractPolynomial& rhs) { (SG&)(*this) += rhs; return *this; }
    /// addition
    AbstractPolynomial operator+(const AbstractPolynomial& rhs) const { auto p = *this; p += rhs; return p; }

    /// unary minus
    const AbstractPolynomial operator-() const { return -(SG&)*this; }
    /// inplace subtraction
    AbstractPolynomial& operator-=(const AbstractPolynomial& rhs) { *this += -rhs; }
    /// subtraction
    const AbstractPolynomial operator-(const AbstractPolynomial& rhs) const { auto p = *this; p -= rhs; return p; }

    /// multiplication
    const AbstractPolynomial operator*(const AbstractPolynomial& rhs) const {
        AbstractPolynomial P;
        for(auto& kv: *this)
            for(auto& kv2: rhs)
                P[kv.first + kv2.first] += kv.second * kv2.second;
        return P;
    }
    /// inplace multiplication
    AbstractPolynomial& operator*=(const AbstractPolynomial& rhs) { *this = (*this) * rhs; return *this; }
    /// scalar multiplication
    AbstractPolynomial& operator*=(const coeff_t& rhs) { for(auto& kv: *this) kv.second *= rhs; return *this; }

    /// exponentiation
    AbstractPolynomial pow(unsigned int e) const {
        if(!e) return AbstractPolynomial(1);
        auto P = *this;
        while(--e) P *= *this;
        return P;
    }

    /////////////////////////////////
    // optional variants as-supported

    /// transform as sum of terms
    template<class X>
    auto transform(const X& x) const -> decltype(x(this->begin()->first,this->begin()->second)) {
        decltype(x(this->begin()->first,this->begin()->second)) P;
        for(auto& kv: *this) P += x(kv.first,kv.second);
        return P;
    }
};

/// convenience typedef for 1D polynomial
template<typename T = double>
using Pol1_t = AbstractPolynomial<T, SemigroupPlus<int>>;
/// convenience typdef for N-dimensional polynomial
template<size_t N, typename T = double>
using Polynomial_t = AbstractPolynomial<T, SGArray_t<N>>;
/// convenience typdef for map-type polynomial
template<typename T = double>
using PolynomialM_t = AbstractPolynomial<T, SGMap_t<>>;
/// convenience typdef for sorted-vector-type polynomial
template<typename T = double>
using PolynomialV_t = AbstractPolynomial<T, SGVec_t<>>;

/// output representation for polynomial
template<class R, class S, typename _X>
std::ostream& operator<<(std::ostream& o, const AbstractPolynomial<R,S,_X>& P) {
    const char* vletters = "xyztuvwabcdefghijklmnopqrsXYZTUVWABCDEFGHIJKLMNOPQRS";

    auto e = P.get();
    o << "( ";
    for(auto& p: e) {
        if(p.second) o << p.second << "*";

        auto v = p.first.get();
        if(v.size()) {
            for(auto& x: v) {
                if(0 <= x.first && x.first < 52) o << vletters[x.first];
                else o << "[" << x.first << "]";
                if(x.second != 1) o << "^" << x.second;
            }
        } else o << "1";
        o << " ";
    }
    o << ")";
    return o;
}

#endif
