/// \file testFiniteGroup.cc Test of finite group code

#include "FiniteGroup.hh"

int main(int, char**) {
    typedef CyclicGroup<2> C2;
    typedef CyclicGroup<3> C3;
    displayGroup<ProductGroup<C2,C3>>();

    array<int,3> A = {4,5,6};
    for(size_t i=0; i<factorial(3); i++) {
        auto P = PermuteArray(i, 3, A);
        printf("%zu]\t%i %i %i\n", i, P[0], P[1], P[2]);
    }
}
