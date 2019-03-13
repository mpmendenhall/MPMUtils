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

    const M11_cayley_t& M11_CT() {
        static M11_cayley_t CT(M11());
        return CT;
    }

    const M12_repr_t M12a = {{3,1,9,0,10,11,6,7,8,2,4,5}};

    const M12_repr_t M12b = {{7,2,3,1,11,9,5,8,0,6,4,10}};

    const M12_genspan_t& M12() {
        static M12_genspan_t M12({M12a,M12b});
        return M12;
    }
}
