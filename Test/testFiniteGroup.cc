/// \file testFiniteGroup.cc Test of finite group code

#include "FiniteGroup.hh"

int main(int, char**) {
    //typedef CyclicGroup<2> C2;
    //typedef CyclicGroup<3> C3;
    //displayGroup<ProductGroup<C2,C3>>();

    typedef AlternatingGroup<4> G;
    for(size_t i=0; i<G::order; i++) {
        auto P = G::permutation(i);
        printf("%zu]\t%i %i %i %i", i, P[0], P[1], P[2], P[3]);
        auto Q = G::invert(P);
        printf("\t\t%i %i %i %i\n", Q[0], Q[1], Q[2], Q[3]);
        if(G::apply(Q,P) != G::identity()) exit(-1);
    }
}
