/// @file GridData.hh Uniformly-gridded interpolable data
// -- Michael P. Mendenhall, LLNL 2021

#ifndef GRIDDATA_HH
#define GRIDDATA_HH

#include <cstdint>
#include <vector>
using std::vector;
#include "lininterp.hh"

/// Uniformly-spaced datapoints
template<class T>
class GridData: public vector<T> {
public:
    /// data type
    typedef T value_t;

    /// Constructor inheritance
    using vector<T>::vector;

    /// linear interpolate to sample position
    double interpolate(fracindex_t s) const { return lininterp(*this, s); }
};

/// Gridded data axis
class GridAxis {
public:
    int64_t s_start = 0;    ///< sample start point [samples from global t0]
    double t0 = 0;  ///< starting time (center of first "bin")
    double dt = 1;  ///< grid spacing

    /// x bin position
    double binX(double i) const { return t0 + i*dt; }

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
