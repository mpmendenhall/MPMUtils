/// \file testUnits.cc Test units definitions

#include "ConfigFactory.hh"

#include "UnitDefs.hh"
#include <stdlib.h>

using namespace Units;

REGISTER_EXECLET(testUnits) {
    printf("%g\n", mile.in(foot));

    printf("%g\n", ampere.in(candela)); // fail!
}
