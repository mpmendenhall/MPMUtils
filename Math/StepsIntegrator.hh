/// \file StepsIntegrator.hh Integration of step functions
// Michael P. Mendenhall, 2019

#include "coerced_lower_bound.hh"
#include <iterator>

/// dereferenced iterator/pointer contents
template<typename it_t>
using itval_t = typename std::iterator_traits<it_t>::value_type;

/// Steps function integration, given x[n] and y[n-1] step function data
template<typename itx_t, typename ity_t, typename x_t>
itval_t<ity_t> integrate_steps(itx_t xbegin, itx_t xend, ity_t ybegin, x_t x0, x_t x1) {

    itval_t<ity_t> s = {};
    if(xbegin == xend || std::next(xbegin) == xend) return s;

    bool reverse = x1 < x0;
    if(reverse) std::swap(x0,x1);

    auto it0 = coerced_lower_bound(x0, xbegin, xend);
    auto it1 = coerced_lower_bound(x1, xbegin, xend);
    auto ity = ybegin + std::distance(xbegin, it0);
    if(it0 == it1) s = (*ity) * (x1 - x0);
    else {
        auto xb = *++it0;
        s = (*ity)*(xb-x0);
        while(it0 != it1) {
            auto xc = *++it0;
            s += (*++ity)*(xc - xb);
            xb = xc;
        }
        s += *(++ity)*(x1 - xb);
    }

    return reverse? s*(-1) : s;
}

/// x,y ranges wrapper
template<typename xit_t, typename yit_t>
struct xydata_t {
    xit_t xbegin;
    xit_t xend;
    yit_t ybegin;
};

/// auto-deduced helper for making x data wrapper
template<typename xit_t, typename yit_t>
xydata_t<xit_t,yit_t> make_xydata(xit_t xbegin, xit_t xend, yit_t ybegin) { return {xbegin, xend, ybegin}; }

/// Cumulative integration along specified points list
template<typename axy_t, typename apo_t>
void integrate_steps_cumulative(axy_t axy, apo_t apo) {
    if(apo.xbegin == apo.xend) return;

    auto it0 = apo.xbegin;
    *apo.ybegin = {};
    while(++apo.xbegin != apo.xend) {
        auto c = *apo.ybegin;
        *(++apo.ybegin) = c + integrate_steps(axy.xbegin, axy.xend, axy.ybegin, *it0, *apo.xbegin);
        it0 = apo.xbegin;
    }
}
