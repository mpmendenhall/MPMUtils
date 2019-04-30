/// \file DivisorCheck.hh Precalculated integer divisor check

#ifndef DIVISORCHECK_HH
#define DIVISORCHECK_HH

#include <array>
using std::array;
#include <cmath>
#include <limits> // for std::numeric_limits
#include <stdio.h>

/// Precalculated fast divisor calculation
template<typename int_t = int64_t>
class DivisorCheck {
public:
    /// unsigned version of integer
    typedef std::make_unsigned_t<int_t> uint_t;

    /// constructor from divisor
    DivisorCheck(int_t dd): d(std::abs(dd)) {

        if(!d) return;

        while(!(d & 1)) {
            d = d >> 1;
            ++j;
        }
        em = ((uint_t)(1) << j) - 1;

        if(d <= 1) { lim = 1; return; }

        auto w = 8*sizeof(int_t) - 1; // number of unsigned bytes
        uint_t m = uint_t(1) << w;
        mul = egcd(d, m)[1] & ~m;
        lim = m / d;
    }

    /// check whether d is divisor of x
    bool divides(int_t x) const {
        if(x & em) return false;
        return (uint_t(std::abs(x))*mul & std::numeric_limits<int_t>::max()) < lim;
    }

    /// display contents
    void display() const {
        printf("%lli * 2^%lli | x <=> x * %llx < %llx\n",
               (long long int)(d),
               (long long int)(j),
               (long long int)mul,
               (long long int)lim);
    }

protected:
    /// internal gcd calculation
    static array<uint_t,3> egcd(uint_t a, uint_t b) {
        if(a) {
            auto gyx = egcd(b % a, a);
            return {gyx[0], gyx[2]-(b/a)*gyx[1], gyx[1]};
        }
        return {b, (uint_t)0, (uint_t)1};
    }

    uint_t j = 0;       ///< number of powers of 2
    uint_t em = 0;      ///< even multipliers bit mask
    uint_t d = 0;       ///< divisor
    uint_t mul = 0;     ///< modular multiplier
    uint_t lim = 0;     ///< comparison limit
};

#endif

