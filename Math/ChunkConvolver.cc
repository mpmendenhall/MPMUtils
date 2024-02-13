/// @file ChunkConvolver.cc

#include "ChunkConvolver.hh"
#include <stdexcept>

void ChunkConvolver::setKernel(const vector<double>& k) {
    N = k.size();
    kern = k;
    auto& P = IFFTWorkspace<plan_t>::get_iffter(2*N);
    auto it = P.v_x.begin();
    for(auto x: k) *(it++) = x;
    for(; it != P.v_x.end(); ++it) *it = 0;
    P.execute();
    kkern.assign(P.v_k.begin(), P.v_k.end());
}

template<typename it_t>
void extract_range_cyclic(it_t begin, it_t end, int i0, size_t N, double* out) {
    int l = end - begin;
    if(i0 < 0) {
        i0 = l - ((-i0) % l);
    } else i0 = i0 % l;

    it_t it = begin + i0;
    for(size_t i = 0; i < N; ++i) {
        if(it == end) it = begin;
        out[i] = *(it++);
    }
}

/*
    kernel size N = 4
    orig_size = 10:
   |abcd|    |    |    |
    1234 1234 12.. tail
    zero:       00 0000
    flat:       22 2222
    wrap:       12 3412

   n_chunks = (10-1)/4 + 1 = 3

   internal convolutions on chunks:
    |1234|0000| repeating...
     uuuu ttt0  repeating...
  abcd ...abcd

   out:
abcd          abc d
   |              |
...1234 1234 12xx x000
        (abc)d     abc
*/

void ChunkConvolver::convolve(const vector<double>& _v_in, vector<double>& v_out) const {
    auto orig_size = _v_in.size();
    if(!orig_size) { v_out.clear(); return; }
    if(!N) throw std::logic_error("Convolution with null-dimension kernel");
    auto final_size = orig_size + N - 1;

    auto& P = IFFTWorkspace<plan_t>::get_iffter(2*N);
    vector<double> vtail(N);    // second half of previous chunk

    // starting boundary contribution
    if(boundaries[0] == BOUNDARY_WRAP) {
        extract_range_cyclic(_v_in.begin(), _v_in.end(), -N, N, P.v_x.data());
        for(int i=N; i < 2*N; ++i) P.v_x[i] = 0;
        do_convolve(P);
        vtail.assign(P.v_x.begin() + N, P.v_x.end());
    }
    if(boundaries[0] == BOUNDARY_FLAT) {
        for(int i=0; i < N; ++i) P.v_x[i] = _v_in[0];
        for(int i=N; i < 2*N; ++i) P.v_x[i] = 0;
        do_convolve(P);
        vtail.assign(P.v_x.begin() + N, P.v_x.end());
    }

    // pad out end for full chunked calculation
    size_t n_chunks = (final_size - 1)/N + 1;
    auto v_in = _v_in;
    v_in.resize(final_size, boundaries[1] == BOUNDARY_FLAT? _v_in.back() : 0.);
    if(boundaries[1] == BOUNDARY_WRAP) {
        extract_range_cyclic(_v_in.begin(), _v_in.end(), 0, v_in.size() - orig_size, v_in.data() + orig_size);
    }
    v_in.resize((n_chunks + 1) * N);
    v_out.resize(n_chunks * N);

    for(size_t c = 0; c < n_chunks; ++c) {
        size_t n0 = c * N;
        int i = 0;
        for(; i < N; ++i) P.v_x[i] = v_in[n0 + i];
        for(; i < 2*N; ++i) P.v_x[i] = 0;
        do_convolve(P);

        for(i=0; i<N; ++i) v_out[n0 + i] = vtail[i] + P.v_x[i];
        vtail.assign(P.v_x.begin() + N, P.v_x.end());
    }

    // truncate to correct size
    v_out.resize(final_size);
}

void ChunkConvolver::do_convolve(workspace_t& P) const {
    P.execute();
    auto it = kkern.begin();
    for(auto& k: P.v_k) k *= *(it++);
    P.etucexe();
}

void ChunkConvolver::setGaussianKernel(double sigma, size_t w) {
    if(!w) w = size_t((sigma + 0.5)*12);

    vector<double> v(w);
    double nrm = 0;
    double s2 = 2*sigma*sigma;
    for(int n=0; n<(int)w; ++n) {
        double x = n - 0.5*(w-1);
        v[n] = exp(-x*x/s2);
        nrm += v[n];
    }
    for(auto& x: v) x /= nrm;
    setKernel(v);
}
