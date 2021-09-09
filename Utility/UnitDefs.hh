/// \file UnitDefs.h Seven-dimensional SI units system
// Michael P. Mendenhall, 2020

#ifndef UNITDEFS_HH
#define UNITDEFS_HH

#include "Rational.hh"
#include "Vec.hh"
#include <stdexcept>

/// namespace for units represented (internally) in the SI system
namespace Units {
    /// Unit dimensions in SI system, rational powers of the 7 SI base units {m, kg, s, A, K, mol, cd}.
    typedef Vec<7,Rational> dimensions_t;

    /// Value with units
    class Unitful: protected dimensions_t {
    public:
        /// default constructor for dimensionless
        explicit Unitful(double v = 0): val(v) { }

        /// constructor
        /// @param v dimensionless multiplier
        /// @param d rational powers of the 7 SI base units
        Unitful(double v, const dimensions_t& d): dimensions_t(d), val(v) { }

        /// constructor from existing unit
        /// @param v dimensionless multiplier
        /// @param u 7-dimensional unit to be multiplied by v
        Unitful(double v, const Unitful& u): dimensions_t(u), val(v*u.val) { }

        /// multiply unitful values
        Unitful& operator*=(const Unitful& v) { (dimensions_t&)(*this) += (dimensions_t&)v; val *= v.val; return *this; }
        /// multiply unitful values
        const Unitful operator*(const Unitful& v) const { auto c = *this; return c *= v; }

        /// multiply unitful values
        Unitful& operator/=(const Unitful& v) { (dimensions_t&)(*this) -= (dimensions_t&)v; val /= v.val; return *this; }
        /// multiply unitful values
        const Unitful operator/(const Unitful& v) const { auto c = *this; return c /= v; }

        /// this item in specified (consistent!) units
        double in(const Unitful& v) const {
            if((const dimensions_t&)(*this) != (const dimensions_t&)v) throw std::logic_error("Inconsistent units");
            return val / v.val;
        }

        double val; ///< value in base units
    };

}

#include "UnitDefs_Base.hh"
#include "UnitDefs_Mechanics.hh"
#include "UnitDefs_EM.hh"

#endif
