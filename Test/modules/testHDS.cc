/// \file testHDS.cc Test of Halfedge Data Structure
// Michael P. Mendenhall, 2019

#include "ConfigFactory.hh"
#include "HalfedgeDS.hh"
#include <stdlib.h>

REGISTER_EXECLET(testHDS) {

    HalfedgeDS H(3);
    H.display(true);

    H.split_all_edges();

    for(auto& f: H.fs)
        if(&f != H.f_outer)
            H.split_corners(f);

    H.display(true);
}
