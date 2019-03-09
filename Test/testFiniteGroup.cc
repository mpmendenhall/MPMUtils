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

    PhiField p{0,1};
    std::cout << p << " -> " << p*p << " -> " << p.inverse() << "\n";

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

    std::cout << Icosahedral::Rs.size() << " icosahedral rotations\n";
    for(auto& m: Icosahedral::Rs) {
        auto t = m.trace(); // (t+1)*(-t+3) = |axis(m)|^2
        std::cout << m << "\n" << (t+1)*(-t+3) << "\n" << Icosahedral::axis(m) << "\t" << Icosahedral::cosTheta(m)  << "\n";
    }

    // fix 31 points, and find 12-point version!
    auto vs = Icosahedral::points(Icosahedral::u12);
    std::cout << vs.size() << " points:\n";
    for(auto& v: vs) std::cout << v << "\n";

    AbstractPolynomial<Rational, SemigroupPlus<int>> Pr({{1,2}, {3,{4,5}}});
    Pr += Rational(1,2);
    std::cout << Pr << Pr.pow(5) << "\n";
}
