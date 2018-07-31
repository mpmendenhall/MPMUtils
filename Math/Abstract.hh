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
// _Semigroup: set, operator *: a,b in S => a*b in s
//
// monoid: _Semigroup with identity element 1 in S, 1*a = a*1 = a for all a in S
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

//template<typename A>
//using array_contents_t = typename std::tuple_element<0,A>::type;

template<typename T>
using array_contents_t = typename std::remove_reference<decltype(std::declval<T&>()[0])>::type;

template<typename M>
using map_key_t = typename std::remove_reference<decltype(std::declval<M&>().begin()->first)>::type;

template<typename M>
using map_value_t = typename std::remove_reference<decltype(std::declval<M&>().begin()->second)>::type;

////////////////////////////////////////////////
////////////////////////////////////////////////

/// Pass-through _Semigroup identification class
template<class G>
class _Semigroup { };

/// output representation for standard-form _Semigroup
template<class G>
std::ostream& operator<<(std::ostream& o, const _Semigroup<G>& s) {
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

/// apply semigroup operator as multiply
template<typename SG, typename T, typename Data>
T& SGmultiply(const SG& o, const Data& d, T& x0) {
    auto ts = o.get();
    for(auto& kv: ts) {
        auto e = kv.second;
        while(e-- > 0) x0 *= d[kv.first];
    }
    return x0;
}

/// apply semigroup operator as add
template<typename SG, typename T, typename Data>
T& SGadd(const SG& o, const Data& d, T& x0) {
    auto ts = o.get();
    for(auto& kv: ts) {
        auto e = kv.second;
        while(e-- > 0) x0 += d(kv.first);
    }
    return x0;
}

////////////////////////////////////////////////

/// Arithmetic type pass-through class
template<typename T>
class _ArithmeticRing { };

/// Arithmetic operations as _Semigroup
template<typename T>
class _Semigroup<_ArithmeticRing<T>> {
public:
    /// numbering type for generator multiplicity
    typedef T num_t;
    /// generator enumeration index --- only one generator!
    typedef int gen_t;
    /// _Semigroup element
    typedef vector<pair<gen_t,num_t>> elem_t;
    /// array size
    static constexpr auto N = 1;

    /// Constructor
    _Semigroup(const T& X = 0): x(X) { }

    /// get standard-form element representation
    elem_t get() const { return {{0,x}}; }
    /// recast to base type
    operator T() const { return x; }

    T x;    ///< arithmetic value

    /// comparison
    bool operator<(const _Semigroup& rhs) const { return x < rhs.x; }
    /// inplace addition
    _Semigroup& operator+=(const _Semigroup& rhs) { x += rhs.x; return *this; }
    /// out-of-place addition
    const _Semigroup operator+(const _Semigroup& rhs) const { return x + rhs.x; }
    /// inplace multiplication
    _Semigroup& operator*=(const _Semigroup& rhs) { x *= rhs.x; return *this; }
    /// out-of-place multiplication
    const _Semigroup operator*(const _Semigroup& rhs) const { return x * rhs.x; }
    /// inplace division
    _Semigroup& operator/=(const _Semigroup& rhs) { x /= rhs.x; return *this; }
    /// out-of-place division
    const _Semigroup operator/(const _Semigroup& rhs) const { return x / rhs.x; }
    /// unary minus
    const _Semigroup operator-() const { return -x; }
};

/// convenience typedef for Arithmetic on T
template<typename T>
using ArithmeticRing_t = _Semigroup<_ArithmeticRing<T>>;

/// output representation for arithmetic value
template<typename T>
std::ostream& operator<<(std::ostream& o, const ArithmeticRing_t<T>& s) {
    o << T(s);
    return o;
}

/////////////////////////////////////////

/// Array-type pass-through tag
template<typename A>
class _SGArray { };

/// Specialization for array-type _Semigroups
template<typename A>
class _Semigroup<_SGArray<A>>: public A {
public:
    /// exponent type from array contents
    typedef array_contents_t<A> num_t;
    /// generator enumeration index
    typedef unsigned int gen_t;
    /// _Semigroup element
    typedef vector<pair<gen_t,num_t>> elem_t;
    /// array size
    static constexpr auto N = std::tuple_size<A>::value;

    /// inherit Array constructors
    using A::A;
    /// default constructor
    _Semigroup(): A() { }
    /// constructor from array; allows curly-brackets init SGarray_t<3>({1,2,3});
    _Semigroup(const A& a): A(a) { }
    /// Costructor for single-variable x_i^n
    _Semigroup(gen_t i, num_t n = 1): A() { assert(i < this->size()); (*this)[i] = n; }

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
    _Semigroup& operator+=(const _Semigroup& rhs) {
        size_t i=0;
        for(auto& e: *this) e += rhs[i++];
        return *this;
    }
    /// out-of-place addition
    const _Semigroup operator+(const _Semigroup& rhs) const { auto s = *this; s += rhs; return s; }

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
using SGArray_t = _Semigroup<_SGArray<array<T,N>>>;

/////////////////////////////////////////

/// sorted-vector type pass-through tag
template<typename V>
class _SGVec: public V { };

/// Specialization for sorted-vector _Semigroups
template<typename V>
class _Semigroup<_SGVec<V>>: protected V {
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
    bool operator<(const _Semigroup& rhs) const { return (V&)(*this) < (V&)(rhs); }

    /// default constructor
    _Semigroup() { }
    /// Costructor for single-variable x_i^n
    _Semigroup(gen_t i, num_t n = 1) { if(n) this->push_back({i,n}); }

    /// get standard-form element representation
    elem_t get() const { return elem_t(*this); }

    /// inplace addition
    _Semigroup& operator+=(const _Semigroup& rhs) {
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
    const _Semigroup operator+(const _Semigroup& rhs) const { auto s = *this; s += rhs; return s; }

    /// remove 0-exponent generators
    void removeNull() {
        this->erase(std::remove_if(this->begin(), this->end(), [](factor_t f){return !f.second;}), this->end());
    }

    /// split negative-exponent generators into separate V
    _Semigroup splitNegative() {
        auto it = std::stable_partition(this->begin, this->end(), [](factor_t f){return f.second > 0;});
        _Semigroup SG;
        SG.assign(it, this->end());
        this->erase(it, this->end());
        return SG;
    }

    /// make this and other relatively prime (and positive), returning common factors
    _Semigroup relPrime(_Semigroup& S) {
        _Semigroup P, ip0, ip1;
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

/// convenience typedef for SGVec of T (default unsigned int)
template<typename K = unsigned int, typename V = int>
using SGVec_t = _Semigroup<_SGVec<vector<pair<K,V>>>>;

/////////////////////////////////////////

/// Map-type pass-through tag
template<typename M>
class _SGMap: public M { };

/// Specialization for map-type _Semigroups
template<typename M>
class _Semigroup<_SGMap<M>>: public M {
public:
    /// base map object
    typedef M map_t;
    /// map key is generator
    typedef map_key_t<M> gen_t;
    /// map value is exponent
    typedef map_value_t<M> num_t;
    /// _Semigroup element
    typedef vector<pair<gen_t,num_t>> elem_t;

    /// inherit map constructors
    using M::M;
    /// default constructor
    _Semigroup() { }
    /// constructor from map
    _Semigroup(const M& m): M(m) { }
    /// Costructor for single-variable x_i^n
    _Semigroup(gen_t i, num_t n = 1) { (*this)[i] = n; }

    /// get standard-form element representation
    elem_t get() const {
        elem_t g;
        for(auto& kv: *this) g.emplace_back(kv);
        return g;
    }

    /// inplace addition
    _Semigroup& operator+=(const _Semigroup& rhs) {
        for(auto& kv: rhs) {
            auto it = this->find(kv.first);
            if(it == this->end()) {
                if(kv.second) this->insert(kv);
            } else {
                it->second += kv.second;
                if(!it->second) this->erase(it);
            }
        }
        return *this;
    }
    /// addition
    const _Semigroup operator+(const _Semigroup& rhs) const { auto s = *this; s += rhs; return s; }
};

/// convenience typedef for SGMap of T (default uint)
template<typename K = unsigned int, typename V = unsigned int>
using SGMap_t = _Semigroup<_SGMap<map<K,V>>>;

////////////////////////////////////////////////
////////////////////////////////////////////////

/// Formal abstract polynomial, with +/- and * operations
// R is a ring with operators +,*, inverse -, default constructor '0'
// S is a Semigroup with operator + for exponent symbols x^i, x^k, x^(i+k)
template<typename R, typename S, typename _X = void>
class AbstractPolynomial: public SGMap_t<S,R> {
public:
    typedef SGMap_t<S,R> SG;
    /// coefficients ring member type
    typedef R coeff_t;
    /// monomials _Semigroup member type
    typedef S monomial_t;
    /// exponent type
    typedef typename S::num_t exp_t;

    /// inherit SGMap_t constructors
    using SG::_Semigroup;
    /// default constructor
    AbstractPolynomial() { }
    /// Costructor for single-variable x_i
    AbstractPolynomial(typename monomial_t::gen_t i) { (*this)[monomial_t(i,1)] = 1; }

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

/// convenience typdef for N-dimensional polynomial
template<size_t N, typename T = double, typename C = ArithmeticRing_t<T>>
using Polynomial_t = AbstractPolynomial<C, SGArray_t<N>>;
/// convenience typdef for map-type polynomial
template<typename T = double, typename C = ArithmeticRing_t<T>>
using PolynomialM_t = AbstractPolynomial<C, SGMap_t<>>;
/// convenience typdef for sorted-vector-type polynomial
template<typename T = double, typename C = ArithmeticRing_t<T>>
using PolynomialV_t = AbstractPolynomial<C, SGVec_t<>>;

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
