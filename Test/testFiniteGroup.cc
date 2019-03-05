/// \file testFiniteGroup.cc Test of finite group code

#include "FiniteGroup.hh"

int main(int, char**) {
    //typedef CyclicGroup<2> C2;
    //typedef CyclicGroup<3> C3;
    //displayGroup<ProductGroup<C2,C3>>();

    for(size_t i=0; i<factorial(4); i++) {
        auto P = SymmetricGroup<4>::permutation(i);
        printf("%zu]\t%i %i %i %i\n", i, P[0], P[1], P[2], P[3]);
    }
}
