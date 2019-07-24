/// \file UUID.cc
// Michael P. Mendenhall, LLNL 2019

#include "UUID.hh"
#include <uuid/uuid.h>

int64_t uuid_64() {
    uuid_t out;
    uuid_generate(out);
    return (*(int64_t*)out) ^ (*((int64_t*)out + 1));
}
