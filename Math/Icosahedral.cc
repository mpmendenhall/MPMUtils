/// \file Icosahedral.cc

#include "Icosahedral.hh"
#include "FiniteGroup.hh"
#include <algorithm>
#include <iostream>
using std::cout;

namespace Icosahedral {

    const PhiField phi{0,1};
    const auto ihp = phi.inverse();
    const PhiField half{{1,2},0};

    const elem_t R10{{-phi/2,   ihp/2,    half,
                       ihp/2,   -half,    phi/2,
                       half,    phi/2,    ihp/2 }};

    const elem_t R58{{ phi/2,   ihp/2,   -half,
                       ihp/2,   half,    phi/2,
                       half,   -phi/2,   ihp/2 }};

    genspan_t _Rs({R10,R58});

    cayley_t _CT(_Rs);

    conjugacy_t _CD(_CT);

    auto renum = _CD.make_renumeration();

    const genspan_t Rs = _Rs.renumerate(renum);
    const cayley_t CT = _CT.renumerate(renum);
    const conjugacy_t CD = _CD.renumerate(renum);
    const size_t nID = *CD.M.find(1)->second.CCs.getClassNum(0).begin();

    vector<vec_t> points(const vec_t& v) {
        vector<vec_t> vv(Rs.getOrder());
        auto it = vv.begin();
        for(auto& M: Rs) *(it++) = Matrix<3,3,SurdSum>(M)*v;
        std::sort(vv.begin(), vv.end());
        vv.erase(std::unique(vv.begin(), vv.end()), vv.end());
        return vv;
    }

    axis_t axis(const elem_t& M) {
        auto t = M.trace();
        if(t == PhiField{3,0}) return {}; // identity matrix
        if(t == PhiField{-1,0}) return {{PhiField(1,0), (M(1,0)+M(0,1)+2)/Rational(4), (M(2,0)+M(0,2)+2)/4}};
        return {{M(2,1)-M(1,2), M(0,2)-M(2,0), M(1,0)-M(0,1)}};
    }
}

void Icosahedral::describe() {
    cout << "\n---------------- Icosahedral Symmetry ----------------\n\n";

    cout << "Starting from two generator rotation matrices:\n\n" << R10 << "\n" << R58 << "\n";
    cout << "(where φ = (1+√5)/2 is the `golden ratio')\n";
    cout << "we build the Icosahedral symmetry point group,\n";
    CD.display();
    cout << "\n";

    cout << "The element of order 1 (#" << nID << ") is the identity transformation:\n" << Rs.element(nID) << "\n";

    cout << "The 15 elements of order 2 are flips by pi around axes passing\n";
    cout << "through the midpoints of opposite icosahedral/dodecahedral edges:\n";
    auto& r15i = CD.M.find(2)->second.CCs.getClassNum(0);
    for(auto i: r15i) cout << "\n#" << i << ":\t" << axis(Rs.element(i));
    cout << "\n\n";

    cout << "The 20 elements of order 3 describe rotations of an icosahedral face,\n";
    cout << "or between 3 faces at a dodecahedron vertex, around axes:\n";
    auto& r20i = CD.M.find(3)->second.CCs.getClassNum(0);
    for(auto i: r20i) cout << "\n#" << i << ":\t" << axis(Rs.element(i));
    cout << "\n\n";

    cout << "Two sets of 12 elements of order 5 describe rotations by 2pi/3 and 4pi/3\n";
    cout << "of a dodecahedral face or icosahedral vertex, around axes:\n";
    auto& r12i = CD.M.find(5)->second.CCs.getClassNum(0);
    for(auto i: r12i) cout << "\n#" << i << ":\t"<< axis(Rs.element(i));
    cout << "\n\n";

    cout << "-------------------------------------------------------\n\n";
}
