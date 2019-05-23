/// \file coerced_lower_bound.hh Helper to coerce value into position in sorted range
// Michael P. Mendenhall, 2019

#include <algorithm>

/// "lower bound" position of x in iterator-bounded range, after coercing x into range.
/// for v.size() > 1, returned iterator and successor will be valid.
template<typename it_t, typename X>
it_t coerced_lower_bound(X& x, it_t it0, it_t it1) {
    // undefined action on zero elements...
    if(it0 == it1) return it0;

    // before start of range?
    if(x <= *it0) {
        x = *it0;
        return it0;
    }

    // after end of range?
    --it1;
    if(*it1 <= x) {
        x = *it1;
        return it1 == it0? it1 : --it1;
    }

    // somewhere in the middle...
    return std::lower_bound(it0, it1, x);
}
