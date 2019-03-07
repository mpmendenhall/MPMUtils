/// \file testFiniteGroup.cc Test of finite group code

#include "FiniteGroup.hh"
#include "Polynomial.hh"
#include "Matrix.hh"
#include "Rational.hh"
#include "Icosahedral.hh"
#include <stdlib.h>
#include <stdio.h>

template<class V>
void pperm(const V& v) { for(auto i: v) printf(" %i",i); }

int main(int, char**) {
    //typedef CyclicGroup<2> C2;
    //typedef CyclicGroup<3> C3;
    //typedef ProductGroup<C2,C3> P23;

    const size_t N = 3;

    typedef AlternatingGroup<N> G;
    for(size_t i=0; i<G::order; i++) {
        auto P = G::element(i);
        printf("%zu]\t", i);
        pperm(P);
        auto Q = G::invert(P);
        printf("\t\t");
        pperm(Q);
        printf("\n");
        if(G::apply(Q,P) != G::identity()) exit(-1);
    }

    std::cout << IcosahedralSymmetry::Rs.size() << " icosahedral rotations\n";
    for(auto& m: IcosahedralSymmetry::Rs) std::cout << m << "\n";

    //auto vs = IcosahedralSymmetry::points({{{ {1},{0},{0} }}});
    auto vs = IcosahedralSymmetry::points({{{ {1},{0},{1} }}});
    std::cout << vs.size() << " points:\n";
    for(auto& v: vs) std::cout << v << "\n";

    AbstractPolynomial<Rational, ArithmeticRing_t<int>> Pr({{1,2}, {3,{4,5}}});
    Pr += Rational(1,2);
    std::cout << Pr << Pr.pow(5) << "\n";
}
