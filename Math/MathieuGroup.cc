/// \file MathieuGroup.cc

#include "MathieuGroup.hh"

namespace MathieuGroup {

    const M11_repr_t a = {{ 0,2,0,1,1,
                            2,1,1,0,2,
                            1,1,1,2,2,
                            0,2,2,2,2,
                            0,2,2,1,0 }};

    const M11_repr_t b = {{ 0,1,1,0,1,
                            2,0,0,1,0,
                            0,0,1,2,2,
                            2,1,0,0,0,
                            0,1,2,2,1 }};

    const M11_genspan_t& M11() {
        static M11_genspan_t M11({a,b});
        return M11;
    }

    const M11_cayley_t& M11_CT() {
        static M11_cayley_t CT(M11());
        return CT;
    }
}
