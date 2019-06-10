/// \file testHDS.cc Test of Halfedge Data Structure
// Michael P. Mendenhall, 2019

#include "HalfedgeDS.hh"
#include "CodeVersion.hh"
#include <stdlib.h>

int main(int, char**) {
    CodeVersion::display_code_version();

    HalfedgeDS H(3);
    H.display(true);

    H.split_all_edges();

    for(auto& f: H.fs)
        if(&f != H.f_outer)
            H.split_corners(f);

    H.display(true);

    return EXIT_SUCCESS;
}
