/// @file zlibWrapper.cc
// -- Michael P. Mendenhall

#include "zlibWrapper.hh"
#include <zlib.h>

void deflate(const void* in, size_t n, vector<char>& vout) {
    auto nout = compressBound(n);
    vout.resize(nout);
    compress(reinterpret_cast<unsigned char*>(vout.data()),
             &nout, reinterpret_cast<const unsigned char*>(in), n);
    vout.resize(nout);
}

size_t inflate(const void* in, size_t n, void* out, size_t nout) {
    uLongf _nout = nout; // assure correct type on 32-bit arch
    uncompress(reinterpret_cast<unsigned char*>(out), &_nout,
               reinterpret_cast<const unsigned char*>(in), n);
    return _nout;
}

