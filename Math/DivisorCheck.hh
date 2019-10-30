/// \file DivisorCheck.hh Precalculated integer divisor check

#ifndef DIVISORCHECK_HH
#define DIVISORCHECK_HH

#include <array>
#include <cmath>
#include <limits> // for std::numeric_limits
#include <type_traits> // for std::make_unsigned
#include <stdio.h>

/// Precalculated fast divisor calculation
template<typename _int_t = int64_t>
class DivisorCheck {
public:
    /// signed version of integer
    typedef _int_t int_t;
    /// unsigned version of integer
    typedef typename std::make_unsigned<int_t>::type uint_t;

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
        printf("%lli * 2^%lli | x <=> !(x & %llx) && x * %llx < %llx\n",
               (long long int)d,
               (long long int)j,
               (long long int)em,
               (long long int)mul,
               (long long int)lim);
    }

protected:
    /// internal gcd calculation
    static std::array<uint_t,3> egcd(uint_t a, uint_t b) {
        if(a) {
            auto gyx = egcd(b % a, a);
            return {gyx[0], gyx[2]-(b/a)*gyx[1], gyx[1]};
        }
        return {b, (uint_t)0, (uint_t)1};
    }

    uint_t d   = 0;     ///< divisor odd component
    uint_t j   = 0;     ///< number of powers of 2
    uint_t em  = 0;     ///< even multipliers bit mask
    uint_t mul = 0;     ///< modular multiplier
    uint_t lim = 0;     ///< comparison limit
};

/*
for w-bit integers (using w = 63 for signed int64 case), let m = 2^w and d > 2 be an odd number.
Greatest common divisor gcd(d,m) = 1 since 2 is not a divisor of d,

thus there exists a:
[1] (a*d) % m  = 1;
so, for any n < m,
[2] d*(a*n % m) == n <==> d | n

For any b, we have:
[3] a*n % m <= b <==> d*(a*n % m) <= b*d

Letting
[4] b := floor((m-1)/d)

so b*d <= m-1 && [1] =>
[5] d*(a*n % m) <= m-1 <==> d*(a*n % m) == n <==> d | n

thus, our divisor check is:
a*n % m <= b <==> d | n

*/

#endif

