/// @file Upsampler.hh Convolution-based upsampling interpolation
// Michael P. Mendenhall, LLNL 2024

#ifndef UPSAMPLER_HH
#define UPSAMPLER_HH

#include "ChunkConvolver.hh"

/// Convolutions of fixed-size kernel against arbitrary length input
class Upsampler: protected ChunkConvolver {
public:
    using ChunkConvolver::boundaries;
    using ChunkConvolver::getKernel;

    /// Constructor
    Upsampler() { for(auto& b: boundaries) b = BOUNDARY_FLAT; }

    /// perform upsampling, output vector of size n_up * input. Okay for vin == vout.
    void upsample(const vector<double>& vin, vector<double>& vout);
    /// get upsampling factor
    size_t get_n_up() const { return n_up; }

    /// Gaussian-tapered truncated sinc kernel (sigma in lobes)
    void set_sinc_interpolator(size_t nup, int nlobes = 8, double sigma = 3.);

    /// normalize kernel to unit sum
    void normalize_kernel(vector<double>& k) const;

protected:
    size_t n_up = 1;    ///< upsampling factor
};

#endif
