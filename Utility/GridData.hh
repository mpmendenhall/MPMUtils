/// @file GridData.hh Uniformly-gridded interpolable data
// -- Michael P. Mendenhall, LLNL 2021

#ifndef GRIDDATA_HH
#define GRIDDATA_HH

#include <cstdint>
#include <vector>
using std::vector;
#include "lininterp.hh"

/// Downsample
template<typename T, typename Tmid = T>
void downsample(vector<T>& v, int d, bool downscale) {
    if(d <= 1 || !v.size()) return;

    auto it = v.begin();
    int n = 0;
    Tmid s = *it;
    for(auto x: v) {
        if(++n == d) {
            n = 0;
            *(it++) = downscale? s/d : s;
            s = x;
        } else s += x;
    }
    v.resize(v.size()/d);
}

/// Uniformly-spaced datapoints
template<class T, typename Tmid = T>
class GridData: public vector<T> {
public:
    /// data type
    typedef T value_t;
    /// internal calculation type
    typedef Tmid calcs_t;

    /// Constructor inheritance
    using vector<T>::vector;

    /// linear interpolate to sample position
    double interpolate(fracindex_t s) const { return lininterp(*this, s); }

    /// Downsample
    virtual void downsample(int d, bool downscale) { ::downsample<T,Tmid>(*this, d, downscale); }
};

/// Gridded data axis
class GridAxis {
public:
    int64_t s_start = 0;    ///< sample start point [samples from global t0]
    double t0 = 0;  ///< starting time (center of first "bin")
    double dt = 1;  ///< grid spacing

    /// x bin position
    double binX(double i) const { return t0 + (s_start + i)*dt; }

    /// shift t0 -> t0 + ds * dt
    void xshift(double ds) { t0 += ds * dt; }
};

/// Grid data with x axis information
template<class GD>
class GridDataWithXAxis: public GD, public GridAxis {
public:
    /// underlying grid data class
    typedef GD GridData_t;
    /// inherit constructors
    using GridData_t::GridData_t;

    /// linear interpolate, relative to t0
    double operator()(double t) const { return GridData_t::interpolate(fracindex_t(t/dt)); }

    /// one-past-end sample position [samples from global t0]
    int64_t s_end() const { return s_start + this->size(); }

    /// set axis info
    void setAxisFrom(const GridAxis& v) { (GridAxis&)*this = v; }
    /// copy data from another GridData
    template<class GD2>
    void copyFrom(const GD2& v) { setAxisFrom(v); this->assign(v.begin(), v.end()); }

    /// Downsample
    void downsample(int d, bool downscale) override {
        if(d <= 1) return;

        dt *= d;
        size_t xstart = (s_start < 0? -s_start : s_start) % d;
        if(xstart && s_start > 0) xstart = d - xstart;
        s_start += xstart;
        s_start /= d;
        if(xstart) {
            if(xstart >= this->size()) {
                this->clear();
                return;
            }
            this->assign(this->begin() + xstart, this->end());
        }
        GridData_t::downsample(d, downscale);
    }
};

#ifdef ENABLE_EXEGETE
#include "EX_VariableNote.hh"
using namespace EX;
/// helper for stringizing GridData
template<typename T>
std::ostream& operator<<(std::ostream& o, const GridData<T>& v) { return o << (const vector<T>&)v; }
#endif

/// Uniformly gridded cumulative curve of waveform
template<class GD>
class GridDataCumulative: public GD {
public:
    /// underlying grid data class
    typedef GD GridData_t;
    /// pointwise values type
    typedef typename GridData_t::value_t value_t;

    /// default constructor
    GridDataCumulative() { }

    /// construct CDF from PDF
    explicit GridDataCumulative(const GridData_t& W): GridData_t(W) { toCDF(); }

    /// construct CDF from PDF: ** destroys input **
    GridDataCumulative(GridData_t& W, int) {
        std::swap((GridData_t&)*this, W);
        toCDF();
    }

    /// inplace conversion from PDF to CDF (offset by -dt/2, +1 point)
    void toCDF() {
        value_t c = 0;
        for(auto& y: *this) {
            auto _y = y;
            y = c;
            c += _y;
        }
        this->push_back(c);
        this->xshift(-0.5);
    }

    /// sort the CDF points => forces monotone increase
    void sort() { std::sort(this->begin(), this->end()); }

    /// quantile inverse: find linearly-interpolated t crossing value between samples
    double quantile_sample(value_t c) const {
        auto idx = locate(c, *this);
        return idx.j == -1? 0 : idx.j == 2? this->size() : double(idx);
    }
};

#endif
