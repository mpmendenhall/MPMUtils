/// \file UnitDefs.h Seven-dimensional SI units system
// Michael P. Mendenhall, 2021

#ifndef UNITDEFS_HH
#define UNITDEFS_HH

#include "Vec.hh"
#include <stdexcept>

/// namespace for units represented (internally) in the SI system
namespace Units {
    /// Unit dimensions in SI system, rational powers of the 7 SI base units {m, kg, s, A, K, mol, cd}.
    typedef Vec<7, int> dimensions_t;

    /// Value with units
    class Unitful: protected dimensions_t {
    public:
        /// default constructor for dimensionless
        explicit Unitful(double v = 0): val(v) { }

        /// constructor
        Unitful(double v, const dimensions_t& d): dimensions_t(d), val(v) { }

        /// constructor multiplying existing unit
        Unitful(double v, const Unitful& u): dimensions_t(u), val(v*u.val) { }

        /// get units of this
        const dimensions_t& units() const { return *this; }
        /// check and enforce units consistency
        void force_consistent(const dimensions_t& d) const { if(units() != d) throw std::logic_error("Inconsistent units"); }

        /// this item in specified (consistent!) units
        double in(const Unitful& v) const { force_consistent(v.units()); return val / v.val; }

        /// add consistent unitful values
        Unitful& operator+=(const Unitful& v) { force_consistent(v.units()); val += v.val; return *this; }
        /// add consistent unitful values
        const Unitful operator+(const Unitful& v) const { auto c = *this; return c += v; }

        /// subtract consistent unitful values
        Unitful& operator-=(const Unitful& v) { force_consistent(v.units()); val -= v.val; return *this; }
        /// subtract consistent unitful values
        const Unitful operator-(const Unitful& v) const { auto c = *this; return c -= v; }

        /// scalar multiply
        Unitful& operator*=(double k) { val *= k; return *this; }
        /// scalar multiply
        const Unitful operator*(double k) const { return {k, *this}; }
        /// scalar divide
        Unitful& operator/=(double k) { val /= k; return *this; }
        /// scalar multiply
        const Unitful operator/(double k) const { return {1./k, *this}; }

        /// multiply unitful values
        Unitful& operator*=(const Unitful& v) { (dimensions_t&)(*this) += (dimensions_t&)v; val *= v.val; return *this; }
        /// multiply unitful values
        const Unitful operator*(const Unitful& v) const { auto c = *this; return c *= v; }

        /// multiply unitful values
        Unitful& operator/=(const Unitful& v) { (dimensions_t&)(*this) -= (dimensions_t&)v; val /= v.val; return *this; }
        /// multiply unitful values
        const Unitful operator/(const Unitful& v) const { auto c = *this; return c /= v; }

        /// square root of unit
        Unitful sqrt() const { Unitful u(::sqrt(val),units()); for(auto& c: u) c /= 2; return u; }
        /// inverse of unit
        Unitful inverse() const { return {1./val, -units()}; }

        double val; ///< value in base units
    };

    /// helper scalar multiply
    inline Unitful operator*(double k, const Unitful& a) { return {k, a}; }
    /// helper scalar inverse
    inline Unitful operator/(double k, const Unitful& a) { return {k, a.inverse()}; }
}

#include "UnitDefs_Base.hh"
#include "UnitDefs_Mechanics.hh"
#include "UnitDefs_EM.hh"

#endif
