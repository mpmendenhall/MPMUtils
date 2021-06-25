/// \file Averager.hh Simple statistics accumulation
// -- Michael P. Mendenhall, 2019

#include <cmath>

/// Weighted average with second moment
template<typename value_t = double, typename weight_t = double>
struct Averager {
    /// Add weighted item
    void add(value_t v, weight_t w = 1) {
        sum_w += w;
        auto vw = v*w;
        sum_wx += vw;
        sum_wxx += v*vw;
    }
    /// add with unity weight
    Averager& operator+=(value_t v) { add(v); return *this; }
    /// add averager
    Averager& operator+=(const Averager& a) {
        sum_w += a.sum_w;
        sum_wx += a.sum_wx;
        sum_wxx += a.sum_wxx;
        return *this;
    }

    /// get average
    value_t average() const { return sum_wx/sum_w; }
    /// get average
    explicit operator value_t() const { return average(); }
    /// get mean square deviation
    value_t variance() const { auto x = average(); return sum_wxx/sum_w - x*x; }
    /// RMS variation
    value_t sigma() const { return sqrt(variance()); }
    /// sqrt(N)-weighted uncertainty
    value_t uncert() const { return sqrt(variance()/sum_w); }

    weight_t sum_w{};   ///< sum of weights
    value_t sum_wx{};   ///< weighted sum w*x
    value_t sum_wxx{};  ///< weighted sum w*x^2
};
