/// @file vecmedian.hh vector median utilities

#include <vector>
using std::vector;
#include <stdexcept>
#include <algorithm>

/// sort and return median
template<typename T>
T sort_median(vector<T>& v) {
    std::sort(v.begin(), v.end());
    return v.at(v.size()/2);
}

/// sort input vector and return average of fraction of median values
template<typename T>
double average_median(vector<T>& v, double centerfrac = 0.33, double* rms = nullptr) {
    if(!v.size() || centerfrac <= 0. || centerfrac >= 1.) throw std::logic_error("median undefined");

    std::sort(v.begin(), v.end());

    int ndiscard = v.size() - std::max(1,int(centerfrac*v.size()));
    int istart = ndiscard/2;
    int iend = v.size()-ndiscard/2;

    double dsum = 0;
    for(int i=istart; i<iend; i++) dsum += v[i];
    dsum /= iend-istart;

    if(rms) {
        *rms = 0;
        for(auto x: v) *rms += (x - dsum)*(x - dsum);
        *rms = sqrt(*rms/v.size());
    }

    return dsum;
}
