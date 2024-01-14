/// @file WeightAverager.hh Utility for recording multivariate weighted-average sums with second moments
#ifndef WEIGHTAVERAGER_HH
#define WEIGHTAVERAGER_HH

#include <cmath>
#include <array>
using std::array;
#include <algorithm>

/// Utility class for weighted sums
template<unsigned int N>
class WeightAverager {
public:
    /// Destructor
    virtual ~WeightAverager() { }

    /// add weighted sum
    void operator+=(const WeightAverager<N>& W) {
        sum_w += W.sum_w;
        for(unsigned int i=0; i<N; i++) {
            sum_wx[i] += W.sum_wx[i];
            sum_wxx[i] += W.sum_wxx[i];
            if(c[i] != W.c[i]) sum_wxx[i] += (c[i] - W.c[i])*(c[i]*W.sum_w - W.sum_wx[i]);
        }
    }

    /// clear entries
    virtual void clear() {
        sum_w = 0;
        x_in.fill(0);
        sum_wx.fill(0);
        sum_wxx.fill(0);
    }

    /// get averaged value
    double get_avg(unsigned int i) const { return sum_wx.at(i)/sum_w; }
    /// get RMS around mean value
    double get_rms(unsigned int i) const {
        double c0 = get_avg(i);
        double u = sum_wxx[i] + (c0 - c[i])*(c[i]*sum_w - sum_wx[i]);
        return (u>0 && sum_w>0)? sqrt(u/sum_w) : 0;
    }

protected:

    /// add weighted term
    void fill_with_weight(double w) {
        sum_w += w;
        for(unsigned int i=0; i<N; i++) {
            sum_wx[i] += w*x_in[i];
            sum_wxx[i] += w*(x_in[i]-c[i])*(x_in[i]-c[i]);
        }
    }

    array<double,N> x_in{{}};   ///< input for each variable
    double sum_w = 0;           ///< sum of weights
    array<double,N> sum_wx{{}}; ///< weighted sum
    array<double,N> c{{}};      ///< central values for second moment calculation
    array<double,N> sum_wxx{{}};///< weighted sum of (x-c)^2
};

#endif
