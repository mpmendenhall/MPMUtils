/// \file testUnits.cc Test units definitions

#include "ConfigFactory.hh"

#include "UnitDefs.hh"
#include <stdlib.h>

using namespace Units;

REGISTER_EXECLET(testUnits) {
    printf("1 mile and 6 inches is %g feet\n", (mile + 6*inch).in(foot));
    printf("1 nautical mile is %g miles\n", nmi.in(mile));
    printf("1 mile is %g ells\n", mile.in(ell));
    printf("1 section is %g acre, or (%g mile)^2\n", section.in(acre), section.sqrt().in(mile));
    printf("1/ms is %g Hz\n", (1/(milli*second)).in(hertz));

    try {
        printf("%g\n", ampere.in(candela)); // fail!
    } catch(...) {
        printf("Cannot convert incompatible units\n");
    }
}
