/// \file Hash64.cc
// -- Michael P. Mendenhall, LLNL 2019

#include "Hash64.hh"
#include <stdint.h>

extern "C" int siphash(const uint8_t *in, const size_t inlen, const uint8_t *k, uint8_t *out, const size_t outlen);

size_t _hash64(const void* dat, size_t n) {
    if(!dat) return 0;
    static const size_t k[2] = {0,0};
    size_t h;
    siphash(reinterpret_cast<const uint8_t*>(dat), n,
            reinterpret_cast<const uint8_t*>(k),
            reinterpret_cast<uint8_t*>(&h), 8);
    return h;
}

size_t chash64(size_t a, size_t b) {
    size_t ab[2] = {a,b};
    return _hash64(ab, 2*sizeof(size_t));
}
