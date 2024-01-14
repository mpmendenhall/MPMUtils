/// @file ModularField.cc

#include "ModularField.hh"
#include <map>
#include <cassert>

const uint_fast8_t* modMulTable(size_t N) {
    assert(N < 256);

    static std::map<size_t, unsigned char*> M;
    auto it = M.find(N);
    if(it != M.end()) return it->second;

    auto t = new unsigned char[N*N];
    for(int i = 0; i < int(N); i++)
        for(int j = 0; j < int(N); j++)
            t[i+N*j] = (i*j)%N;
    return (M[N] = t);
}
