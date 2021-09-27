/// \file testFiniteGroup.cc Test of finite group code

#include "ConfigFactory.hh"
#include "GlobalArgs.hh"

#include "JankoGroup.hh"
#include "MathieuGroup.hh"
#include "CyclicGroup.hh"
#include "Stopwatch.hh"
#include <stdlib.h>

REGISTER_EXECLET(testFiniteGroup) {

    int n = 999;
    optionalGlobalArg("n", n, "number of tests");

    if(n) {
        printf("\n\n\n----------- C_6 -------------\n\n");

        Stopwatch w; // ~0.4 ms

        typedef CyclicGroup<6> C6;
        CayleyTable<C6> CT_C6;
        ConjugacyDecomposition<CayleyTable<C6>> GGD_C6;
        GGD_C6.display();

        typedef SymmetricGroup<4> S4;
        ConjugacyDecomposition<S4> GGD_S4;
        GGD_S4.display();
        for(auto& P: S4()) std::cout << P << "\n";

        typedef SymmetricGroup<5> S5;
        ConjugacyDecomposition<S5> GGD_S5;
        GGD_S5.display();
    }

    if(n>1) {
        printf("\n\n\n----------- M_11 -------------\n\n");
        Stopwatch w; // ~0.29 s permutation, ~0.98 s matrix
        MathieuGroup::M11_conj().display();
    }

    if(n>2) {
        printf("\n\n\n----------- M_21 -------------\n\n");
        Stopwatch w; // ~2.34 s
        ConjugacyDecomposition<MathieuGroup::M21_genspan_t> CD_M21(MathieuGroup::M21());
        CD_M21.display();
    }

    if(n>3) {
        printf("\n\n\n----------- M_12 -------------\n\n");
        Stopwatch w; // ~47 s
        ConjugacyDecomposition<MathieuGroup::M12_genspan_t> CD_M12(MathieuGroup::M12());
        CD_M12.display();
    }

    if(n>4) {
        printf("\n\n\n----------- M_11 Cayley Table -------------\n\n");
        {
            Stopwatch w; // ~57 s matrix, ~23 s permutation
            MathieuGroup::M11_CT();
        }

        Stopwatch w; // ~60 ms using precalculated Cayley Table
        OrdersDecomposition<MathieuGroup::M11_cayley_t> OD_M11CT(MathieuGroup::M11_CT());
        OD_M11CT.display();
    }

    if(n>5) {
        printf("\n\n\n----------- J_1 -------------\n\n");
        Stopwatch w; // ~598 s permutation (to order 5 in 31.3 s), ~700 s matrix
        ConjugacyDecomposition<JankoGroup::J1_pgenspan_t> CD_J1(JankoGroup::J1p());
        //ConjugacyDecomposition<JankoGroup::J1_mgenspan_t> CD_J1(JankoGroup::J1m());
        CD_J1.display();
    }

    if(n>6) {
        printf("\n\n\n----------- M_22 -------------\n\n");
        Stopwatch w; // ~1815 s
        ConjugacyDecomposition<MathieuGroup::M22_genspan_t> CD_M22(MathieuGroup::M22());
        CD_M22.display();
    }
}
