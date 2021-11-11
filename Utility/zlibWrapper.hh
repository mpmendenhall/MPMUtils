/// \file zlibWrapper.hh Simple wrapper on zlib deflate/inflate compression
// -- Michael P. Mendenhall

#ifndef ZLIBWRAPPER_HH
#define ZLIBWRAPPER_HH

#include <vector>
using std::vector;
#include <cstddef> // for size_t

/// compress n bytes from in to output vector
void deflate(const void* in, size_t n, vector<char>& out);
/// decompress n bytes from input to (not undersized!) output; return actual nout
size_t inflate(const void* in, size_t n, void* out, size_t nout);

#endif
