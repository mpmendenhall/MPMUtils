/// @file testAverager.cc Validate variance-tracking averager
// -- Michael P. Mendenhall, 2021

#include "ConfigFactory.hh"
#include "Averager.hh"

/// old-style averager with sum w*x, sum w*x*x
template<typename value_t = double, typename weight_t = double>
struct _Averager {
    /// Add weighted item
    void add(value_t v, weight_t w = 1) {
        sum_w += w;
        auto vw = v*w;
        sum_wx += vw;
        sum_wxx += v*vw;
    }
    /// add with unity weight
    _Averager& operator+=(value_t v) { add(v); return *this; }
    /// add averager
    _Averager& operator+=(const _Averager& a) {
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
    /// uncertainty squared
    value_t uncert2() const { return variance()/sum_w; }

    /// print info to stdout
    void display() const { printf("mu = %g, sigma = %g (w = %g)\n", average(), sigma(), sum_w); }

    weight_t sum_w{};   ///< sum of weights
    value_t sum_wx{};   ///< weighted sum w*x
    value_t sum_wxx{};  ///< weighted sum w*x^2
};


REGISTER_EXECLET(testAverager) {
    Averager<> A, B;
    _Averager<> _A, _B;

    for(double i = 1; i < 5; ++i) {
        A += i;
        A.add(i + 0.5, 0.5);
        B += 3+i;
        A.display();

        _A += i;
        _A.add(i + 0.5, 0.5);
        _B += 3+i;
    }

    printf("--------\n");
    A.display(); _A.display();
    A += B;
    _A += _B;
    A.display(); _A.display();
}
