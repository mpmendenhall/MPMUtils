/// @file vecintegrate.hh templatized vector sum/integral utilities

#include <numeric> // for std::accumulate
#include <vector>
#include <cmath>
using std::vector;

/// sum element range, inclusive (zero outside vector range); set n to number of items summed
template<typename T>
float sum_range(const vector<T>& v, int r0, int r1, int* n = nullptr) {
    if(n) *n = 0;
    r0 = r0 >= 0? r0 : 0;
    if(!(r0 < (int)v.size())) return 0;
    r1 = r1 < (int)v.size()? r1 : v.size()-1;
    if(!(r0 <= r1)) return 0;
    if(n) *n = r1 - r0 + 1;
    return std::accumulate(&v[r0], (&v[r1])+1, 0);
}

/// integrate linearly interpolated range between (fractional) sample numbers r0, r1; set n to actual range
template<typename T>
float integrate_lininterp_range(const vector<T>& v, float r0, float r1, float* n = nullptr) {
    int i0 = ceil(r0);
    float d0 = i0-r0;
    int i1 = floor(r1);
    float d1 = r1-i1;
    int nn = 0;
    float s = (i0 <= i1)? sum_range<T>(v, i0, i1, &nn) : 0.;
    if(n) *n = nn; // .. close enough! TODO refine

    if(i0 >= 0 && i0 < (int)v.size()) s -= (1-d0)*(1-d0)/2*v[i0];
    if(i0 >= 1 && i0 <= (int)v.size()) s += d0*d0/2*v[i0-1];
    if(i1 >= 0 && i1 < (int)v.size()) s -= (1-d1)*(1-d1)/2*v[i1];
    if(i1 >= -1 && i1 < (int)v.size()-1) s += d1*d1/2*v[i1+1];

    return s;
}
