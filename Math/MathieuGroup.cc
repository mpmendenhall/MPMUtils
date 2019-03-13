/// \file MathieuGroup.cc

#include "MathieuGroup.hh"

namespace MathieuGroup {

    const M11_repr_t M11a = {{ 0,2,0,1,1,
                               2,1,1,0,2,
                               1,1,1,2,2,
                               0,2,2,2,2,
                               0,2,2,1,0 }};

    const M11_repr_t M11b = {{ 0,1,1,0,1,
                               2,0,0,1,0,
                               0,0,1,2,2,
                               2,1,0,0,0,
                               0,1,2,2,1 }};

    const M11_genspan_t& M11() {
        static M11_genspan_t M11({M11a,M11b});
        return M11;
    }

    const M11_conj_t& M11_conj() {
        static M11_conj_t C(M11());
        return C;
    }

    const M11_cayley_t& M11_CT() {
        static M11_cayley_t CT(M11());
        return CT;
    }

    //------------------------------------

    const M12_repr_t M12a = {{3,1,9,0,10,11,6,7,8,2,4,5}};
    const M12_repr_t M12b = {{7,2,3,1,11,9,5,8,0,6,4,10}};
    const M12_genspan_t& M12() {
        static M12_genspan_t M12({M12a,M12b});
        return M12;
    }

    //-------------------------------------

    const M21_repr_t M21a = {{1,0,2,5,6,3,4,11,13,14,16,7,18,8,9,15,10,17,12,19,20}};
    const M21_repr_t M21b = {{0,2,4,1,3,7,9,12,5,15,6,17,8,19,13,10,18,11,16,20,14}};
    const M21_genspan_t& M21() {
        static M21_genspan_t M21({M21a,M21b});
        return M21;
    }

    //-------------------------------------

    const M22_repr_t M22a = {{12,7,15,11,4,21,16,1,9,8,13,3,0,10,14,2,6,17,18,19,20,5}};
    const M22_repr_t M22b = {{21,17,20,12,11,10,14,13,8,7,6,4,1,19,5,15,18,3,16,9,0,2}};
        const M22_genspan_t& M22() {
        static M22_genspan_t M22({M22a,M22b});
        return M22;
    }
}
