/// \file MCTAL_Axis.hh Axis in an "MCTAL" MCNP tallies file
// Michael P. Mendenhall, LLNL 2021

#ifndef MCTAL_AXIS_HH
#define MCTAL_AXIS_HH

#include "MCTAL_Includes.hh"

/// Axis basic info without bin values
class MCTAL_Axis {
public:
    explicit MCTAL_Axis(const string& t = "Axis"): title(t) { }

    /// read from file
    void load(const string& cs, istream& i, bool to_endline = true);
    /// print summary info to stdout
    void display() const;

    string title;           ///< axis title
    int nbins = 0;          ///< number of bins, including total if present
    size_t stride = 1;      ///< data stride on this axis

    /// special bin type flag
    enum bintype_t {
        BINTP_NONE = ' ',   ///< no special binning state
        BINTP_TOTAL = 'T',  ///< if there is a Total bin
        BINTP_CUM = 'C'     ///< if Cumulative binning is used
    } bintype = BINTP_NONE;

    /// bin value access
    virtual double operator()(size_t i) const { return i; }
    /// number of bin value entries
    virtual size_t nvals() const { return nbins; }
    /// get bins as double vector
    virtual vector<double> toVec() const;
    /// print numbers to stdout
    void showbins() const;

    /// binning interpretation flag: bin boundaries or center point
    bool is_bin_lowedge = false;
};

/// Double-valued axis binning (Time, Energy, Angle)
class MCTAL_AxBins: public MCTAL_Axis, public vector<double> {
public:
    using MCTAL_Axis::MCTAL_Axis;

    /// read from file
    void load(const string& cs, istream& i);
    /// bin value access
    double operator()(size_t i) const override { return at(i); }
    /// number of bin value entries
    size_t nvals() const override { return size(); }
    /// get bins as double vector
    vector<double> toVec() const override { return *this; }
};

/// Integer-valued axis (e.g. F surface identifiers)
class MCTAL_IntAx: public MCTAL_Axis, public vector<int> {
public:
    using MCTAL_Axis::MCTAL_Axis;

    /// read from file
    void load(istream& i);
    /// bin value access
    double operator()(size_t i) const override { return at(i); }
    /// number of bin value entries
    size_t nvals() const override { return size(); }
};

/// axis identifiers
enum tallyax_id_t {
    AXIS_T = 0,
    AXIS_E = 1,
    AXIS_C = 2,
    AXIS_M = 3,
    AXIS_S = 4,
    AXIS_U = 5,
    AXIS_D = 6,
    AXIS_F = 7,
    AXIS_END = 8
};

/// axes iteration helper
inline tallyax_id_t&
operator++(tallyax_id_t& p) { return (p = tallyax_id_t(p+1)); }

#endif
