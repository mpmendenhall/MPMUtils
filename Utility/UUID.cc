/// @file UUID.cc
// -- Michael P. Mendenhall, LLNL 2019

#include "UUID.hh"
#include <uuid/uuid.h>

template<typename T>
int64_t* uuid_reparse(T* u) { return reinterpret_cast<int64_t*>(u); }

int64_t uuid_64() {
    uuid_t out;
    uuid_generate(out);
    return (*uuid_reparse(out)) ^ (*(uuid_reparse(out) + 1));
}
