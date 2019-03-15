/// \file testFiniteGroup.cc Test of finite group code

#include "FiniteGroup.hh"
#include "Polynomial.hh"
#include "Matrix.hh"
#include "Rational.hh"
#include "JankoGroup.hh"
#include "MathieuGroup.hh"
#include "PermutationGroup.hh"
#include "Stopwatch.hh"
#include <stdlib.h>

int main(int argc, char** argv) {

    int n = 4;
    if(argc > 1) n = atoi(argv[1]);

    if(n) {
        Stopwatch w; // ~0.4 ms

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
    }

    if(n>1) {
        Stopwatch w; // ~0.98 s
        MathieuGroup::M11_conj().display();
    }

    if(n>2) {
        Stopwatch w; // ~2.4 s
        ConjugacyDecomposition<MathieuGroup::M21_genspan_t> CD_M21(MathieuGroup::M21());
        CD_M21.display();
    }

    if(n>3) {
        Stopwatch w; // ~53 s
        ConjugacyDecomposition<MathieuGroup::M12_genspan_t> CD_M12(MathieuGroup::M12());
        CD_M12.display();
    }

    if(n>4) {
        {
            Stopwatch w; // ~52 s
            MathieuGroup::M11_CT();
        }

        Stopwatch w; // ~65 ms using precalculated Cayley Table
        OrdersDecomposition<MathieuGroup::M11_cayley_t> OD_M11CT(MathieuGroup::M11_CT());
        OD_M11CT.display();
    }

    if(n>5) {
        Stopwatch w; // ~700 s
        ConjugacyDecomposition<JankoGroup::J1_genspan_t> CD_J1(JankoGroup::J1());
        CD_J1.display();
    }

    if(n>6) {
        Stopwatch w; // ~2120 s
        ConjugacyDecomposition<MathieuGroup::M22_genspan_t> CD_M22(MathieuGroup::M22());
        CD_M22.display();
    }

    return EXIT_SUCCESS;
}
