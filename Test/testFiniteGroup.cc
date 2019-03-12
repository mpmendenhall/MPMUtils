/// \file testFiniteGroup.cc Test of finite group code

#include "FiniteGroup.hh"
#include "Polynomial.hh"
#include "Matrix.hh"
#include "Rational.hh"
#include "Icosahedral.hh"
#include "JankoGroup.hh"
#include "MathieuGroup.hh"
#include <stdlib.h>
#include <stdio.h>

template<class V>
void pperm(const V& v) { for(auto i: v) printf(" %i",i); }

int main(int, char**) {

    std::cout << Icosahedral::Rs.getOrder() << " icosahedral rotations\n";
    for(auto& m: Icosahedral::Rs) {
        auto t = m.trace(); // (t+1)*(-t+3) = |axis(m)|^2
        std::cout << m << "\n" << (t+1)*(-t+3) << "\n" << Icosahedral::axis(m) << "\t" << Icosahedral::cosTheta(m)  << "\n";
    }

    ConjugacyDecomposition<Icosahedral::cayley_t> GGD(Icosahedral::CT);
    GGD.display();

    typedef CyclicGroup<6> C6;
    CayleyTable<C6> CT_C6;
    ConjugacyDecomposition<CayleyTable<C6>> GGD_C6;
    GGD_C6.display();

    typedef SymmetricGroup<4> S4;
    ConjugacyDecomposition<S4> GGD_S4;
    GGD_S4.display();

    typedef SymmetricGroup<5> S5;
    ConjugacyDecomposition<S5> GGD_S5;
    GGD_S5.display();

    OrdersDecomposition<JankoGroup::J1_genspan_t> OD_J1(JankoGroup::J1());
    OD_J1.display();

    //ConjugacyDecomposition<JankoGroup::J1_genspan_t> GGD_J1(JankoGroup::J1());

    OrdersDecomposition<MathieuGroup::M11_genspan_t> OD_M11(MathieuGroup::M11());
    OD_M11.display();

    OrdersDecomposition<MathieuGroup::M11_cayley_t> OD_M11CT(MathieuGroup::M11_CT());
    OD_M11CT.display();

    ConjugacyDecomposition<MathieuGroup::M11_cayley_t> Conj_M11(MathieuGroup::M11_CT());
    Conj_M11.display();

    return EXIT_SUCCESS;

    // fix 31 points, and find 12-point version!
    auto vs = Icosahedral::points(Icosahedral::u12);
    std::cout << vs.size() << " points:\n";
    for(auto& v: vs) std::cout << v << "\n";

    AbstractPolynomial<Rational, SemigroupPlus<int>> Pr({{1,2}, {3,{4,5}}});
    Pr += Rational(1,2);
    std::cout << Pr << Pr.pow(5) << "\n";
}
