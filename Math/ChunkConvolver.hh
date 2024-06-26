/// @file ChunkConvolver.hh Convolutions of fixed-size kernel against arbitrary length input
// Michael P. Mendenhall, LLNL 2023

#ifndef CHUNKCONVOLVER_HH
#define CHUNKCONVOLVER_HH

#include "FFTW_Convolver.hh"

/// Convolutions of fixed-size kernel against arbitrary length input
class ChunkConvolver {
public:

    /// boundary condition options
    enum boundary_t {
        BOUNDARY_0,         ///< zero outside boundary edges
        BOUNDARY_FLAT,      ///< repeat value of first/last point
        BOUNDARY_WRAP       ///< wrap points from opposite end of data
    };
    /// boundary conditions to apply at start and end
    boundary_t boundaries[2] = {BOUNDARY_0, BOUNDARY_0};

    /// Set convolution kernel
    void setKernel(const vector<double>& k);
    /// Perform convolution, placing result (vin.size() + kernel.size() - 1) in vout
    void convolve(const vector<double>& vin, vector<double>& vout) const;
    /// get real-space kernel size
    size_t kernsize() const { return N; }
    /// get realspace kernel
    const vector<double>& getKernel() const { return kern; }

    /// centered Gaussian kernel (default window width if set to 0)
    void setGaussianKernel(double sigma, size_t w = 0);

protected:
    /// real-to-complex transform planner type
    typedef R2CPlan<double> plan_t;
    /// FFT workspace
    typedef IFFTWorkspace<plan_t> workspace_t;

    /// convolve P.v_in with kkern
    void do_convolve(workspace_t& P) const;
    /// fill pre-data points into size of (initially zero'd) v_in
    void prepoints(const vector<double>& _v_in, vector<double>& v_in) const;
    /// append n post-data points
    void postpoints(const vector<double>& _v_in, vector<double>& v_in, size_t n) const;
    /// convolution on pre-padded input vector
    void _convolve(vector<double>& v_in, vector<double>& v_out, size_t orig_size) const;

    int N = 0;                      ///< real-space kernel size
    vector<double> kern;            ///< real-space kernel
    vector<plan_t::scplx_t> kkern;  ///< k-space kernel
};

#endif
