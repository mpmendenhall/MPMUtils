/// @file "PointSelector.hh" Multidimensional hierarchical point generator

#ifndef POINTSELECTOR_HH
#define POINTSELECTOR_HH

#include <Math/QuasiRandom.h>
#include <iostream>
#include <vector>
using std::vector;

/// Multidimensional hierarchical point selection
class PointSelector {
public:
    typedef vector<double> vec_t;   ///< convenience shorthand

    /// Add partitioned subgroup of N elements, to be sampled npts times
    void addPart(size_t N, size_t npts);
    /// skip to enumerated coordinate
    void skipTo(size_t i);
    /// generate next coordinate
    vec_t next();
    /// print debugging info to stdout
    void display() const;
    /// get number of points in full cycle
    size_t nCycle() const { return parts.size()? parts[0].npts * parts[0].Nsub : 0; }

    size_t subgroup;        ///< indicates modified subgroup on next()

    /// partitioning of fit axes
    struct axpart {
        /// Default constructor
        axpart(): N(0), npts(0) { }
        /// Constructor, with number of elements
        axpart(size_t _N, size_t _npts): N(_N), npts(_npts), QRNG(N) { }

        size_t N;           ///< number of items on this axis
        size_t npts;        ///< number of points to generate at this level
        size_t Nsub = 1;    ///< number of points for sub-groupings

        ROOT::Math::QuasiRandomNiederreiter QRNG;   ///< quasirandom distribution generator
        size_t QRNGn = 0;                           ///< number of points pulled from QRNG
    };

protected:

    vector<axpart> parts;   ///< partioning of N for sub-calculations
    size_t Ntot = 0;        ///< total number of dimensions
    vec_t v0;               ///< previously-generated point

    friend std::ostream& operator<< (std::ostream &o, const PointSelector& NM);
    friend std::istream& operator>> (std::istream &i, PointSelector& NM);
};

/// serialize to output
std::ostream& operator<<(std::ostream& o, const PointSelector& p);
/// deserialize (requires correct dimension to already be set)
std::istream& operator>>(std::istream& i, PointSelector& p);

#endif
