/// \file LinLstsq.hh Simple least-squares y = a+b*x fitter
// Michael P. Mendenhall, 2018

#include <utility> // for std::pair

/// Least squares line fit y = a + b*x
template<class U, class V>
std::pair<double,double> linlsq(const U& x, const V& y) {

    assert(x.size() <= y.size());
    if(y.size() < 2) return {0,0};

    auto itx = x.begin();
    auto ity = y.begin();
    double sx = 0, sy = 0, sxx = 0, sxy = 0;
    size_t n = 0;
    do {
        sx += *itx;
        sy += *ity;
        sxx += (*itx)*(*itx);
        sxy += (*itx)*(*ity);
        n++;
        ++itx;
    } while(++ity != y.end());

    auto d = n*sxx  - sx*sx;
    if(!d) return {0,0};
    double a = (sy*sxx - sx*sxy) / d;
    double b = (n*sxy  - sx*sy) / d;
    return {a,b};
}
