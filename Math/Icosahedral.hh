/// \file Icosahedral.hh Icosahedral symmetry manipulations
// Michael P. Mendenhall, 2019

#ifndef ICOSAHEDRAL_HH
#define ICOSAHEDRAL_HH

#include "Matrix.hh"
#include "SurdField.hh"

/// field of a+b*phi, phi = (1+sqrt(5))/2
class PhiField: public std::pair<Rational,Rational> {
public:
    /// convenience for parent class
    typedef std::pair<Rational,Rational> super;
    /// inherit constructors
    using super::pair;

    /// convert to SurdSum
    operator SurdSum() const { return SurdSum(first + second/Rational(2)) + SurdSum(second/Rational(2))*SurdSum::sqrt(5); }

    /// check if 0
    operator bool() const { return first || second; }
    /// equality
    bool operator==(const PhiField& S) const { return (super&)*this == S; }
    /// inequality
    bool operator!=(const PhiField& S) const { return !(*this == S); }

    /// unary minus
    const PhiField operator-() const { return {-first,-second}; }
    /// inplace addition
    PhiField& operator+=(const PhiField& r) { first += r.first; second += r.second; return *this; }
    /// inplace addition of Rational (also picks up int)
    PhiField& operator+=(const Rational& r) { first += r; return *this; }
    /// addition (automatic type detection)
    template<class T>
    const PhiField operator+(const T& R) const { auto c = *this; return c += R; }
    /// inplace subtraction (automatic type detection)
    template<class T>
    PhiField& operator-=(const T& R) { return *this += -R; }
    /// subtraction (automatic type detection)
    template<class T>
    const PhiField operator-(const T& R) const { return *this + -R; }

    /// invert this = 1/this
    void invert() {
        if(!*this) throw std::domain_error("Refuse to divide by 0!");
        auto x = first*Rational(2) + second;
        if(!x) { second = Rational(4,5)/second; first = -second/Rational(2); }
        else { x = x*x-Rational(5)*second; (super&)*this = {(first+second)*Rational(4)/x, second*Rational(-4)/x}; }
    }
    /// inverse 1/this
    PhiField inverse() const { auto i = *this; i.invert(); return i; }

    /// inplace multiplication by PhiField
    PhiField& operator*=(const PhiField& R) { (super&)*this = {first*R.first + second*R.second, first*R.second + second*R.first + second*R.second}; return *this; }
    /// inplace multiplication by Rational (also picks up int)
    PhiField& operator*=(const Rational& R) { first *= R; second *= R; return *this; }
    /// out-of-place multiplication (automatic type detection)
    template<class T>
    const PhiField operator*(const T& R) const { auto c = *this; return c *= R; }

    /// inplace division
    PhiField& operator/=(const PhiField& R) { return (*this) *= R.inverse(); }
    /// inplace rational division, also picks up int
    PhiField& operator/=(const Rational& R) { if(!R) throw std::range_error("Refuse to divide by 0!"); first /= R; second /= R; return *this; }
    /// out-of-place division (automatic type detection)
    template<class T>
    const PhiField operator/(const T& R) const { auto q = *this; return q /= R; }
};

/// output representation for rational fraction
std::ostream& operator<<(std::ostream& o, const PhiField& r);

/// Information about icosahedral symmetry point group
namespace Icosahedral {
    /// Symmetry group element
    typedef Matrix<3,3,PhiField> elem_t;
    /// Rotation axis type
    typedef Vec<3,PhiField> axis_t;
    /// Vector operated on by group
    typedef Vec<3,SurdSum> vec_t;

    // generators for all Rs
    extern const elem_t R10;    ///< one generator
    extern const elem_t R58;    ///< another generator

    /// All 60 rotation matrices in icosahedral point group
    extern const vector<elem_t> Rs;

    //extern const SurdSum phi;   ///< golden ratio (sqrt(5)+1)/2;
    //extern const SurdSum ihp;   ///< 1/phi = (sqrt(5)-1)/2 = phi-1

    // special unit vectors with reduced symmetries
    extern const vec_t u12; ///< dodecahedral face center
    extern const vec_t u20; ///< icosahedral face center
    extern const vec_t u30; ///< dodecahedral/icosahedral edge center

    /// apply all rotations to vector, eliminating duplicates
    vector<vec_t> points(const vec_t& v);

    /// cos theta for rotation
    inline SurdSum cosTheta(const elem_t& M) { return (M.trace()-1)/2; }

    /// (non-normalized) rotation axis for matrix
    axis_t axis(const elem_t& M);
}

#endif
