/// \file testPolynomial.cc Test of polynomial manipulation

#include "Monomial.hh"
#include <array>
using std::array;
#include <stdlib.h>

typedef Monomial<array<int,3>,float> Mxyz;

int main(int, char**) {

    Mxyz m(5,{1,2,3});
    std::cout << m << "\n";

    return EXIT_SUCCESS;
}
