/// \file JankoGroup.cc

#include "JankoGroup.hh"
#include <stdio.h>

namespace JankoGroup {

    J1_repr_t makeY() {
        J1_repr_t M;
        for(int i=1; i<7; i++) M(i-1,i) = 1;
        M(6,0) = 1;
        return M;
    }

    const J1_repr_t Y = makeY();
    const J1_repr_t Z = {{-3, 2,-1,-1,-3,-1,-3,
                          -2, 1, 1, 3, 1, 3, 3,
                          -1,-1,-3,-1,-3,-3, 2,
                          -1,-3,-1,-3,-3, 2,-1,
                          -3,-1,-3,-3, 2,-1,-1,
                           1, 3, 3,-2, 1, 1, 3,
                           3, 3,-2, 1, 1, 3, 1 }};

    const J1_genspan_t& J1() {
        //printf("Generating J1 group elements...\n");
        static J1_genspan_t J1({Y,Z});
        return J1;
    }
}
