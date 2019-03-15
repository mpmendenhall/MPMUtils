/// \file Icosahedral.cc
// Michael P. Mendenhall, 2019

#include "Icosahedral.hh"
#include <algorithm>
#include <iostream>
using std::cout;

namespace Icosahedral {

    const PhiField phi{0,1};
    const auto ihp = phi.inverse();
    const PhiField half{{1,2},0};

    const elem_t Ra{{-phi/2,   ihp/2,    half,
                      ihp/2,   -half,    phi/2,
                      half,    phi/2,    ihp/2 }};

    const elem_t Rb{{ phi/2,   ihp/2,   -half,
                      ihp/2,   half,    phi/2,
                      half,   -phi/2,   ihp/2 }};

    const GeneratorsConjugacy<groupop_t> IC({Ra,Rb});
    const genspan_t Rs = IC.Rs;
    const cayley_t CT = IC.CT;
    const conjugacy_t CD = IC.CD;

    vector<vec_t> points(const vec_t& v) {
        vector<vec_t> vv(Rs.getOrder());
        auto it = vv.begin();
        for(auto& M: Rs) *(it++) = Matrix<3,3,SurdSum>(M)*v;
        std::sort(vv.begin(), vv.end());
        vv.erase(std::unique(vv.begin(), vv.end()), vv.end());
        return vv;
    }
}

void Icosahedral::describe() {
    cout << "\n---------------- Icosahedral Symmetry ----------------\n\n";

    cout << "Starting from two generator rotation matrices:\n\n" << Ra << "\n" << Rb << "\n";
    cout << "(where φ = (1+√5)/2 is the `golden ratio')\n";
    cout << "we build the Icosahedral symmetry point group,\n";
    CD.display();
    cout << "\n";

    cout << "The element of order 1 (#" << nID << ") is the identity transformation:\n" << Rs.element(nID) << "\n";

    cout << "The 15 elements of order 2 are flips by pi around axes passing\n";
    cout << "through the midpoints of opposite icosahedral/dodecahedral edges:\n";
    auto& r15i = CD.M.find(2)->second.CCs.getClassNum(0);
    for(auto i: r15i) cout << "\n#" << i << ":\t" << R3axis(Rs.element(i));
    cout << "\n\n";

    cout << "The 20 elements of order 3 describe rotations of an icosahedral face,\n";
    cout << "or between 3 faces at a dodecahedron vertex, around axes:\n";
    auto& r20i = CD.M.find(3)->second.CCs.getClassNum(0);
    for(auto i: r20i) cout << "\n#" << i << ":\t" << R3axis(Rs.element(i));
    cout << "\n\n";

    cout << "Two sets of 12 elements of order 5 describe rotations by 2pi/3 and 4pi/3\n";
    cout << "of a dodecahedral face or icosahedral vertex, around axes:\n";
    auto& r12i = CD.M.find(5)->second.CCs.getClassNum(0);
    for(auto i: r12i) cout << "\n#" << i << ":\t"<< R3axis(Rs.element(i));
    cout << "\n\n";

    cout << "We can associate dodecahedral faces f with vertices v (same association\n";
    cout << "vice-versa for icosahedra) by finding combinations vfvf = I:\n\n";
    for(auto f: r12i) {
        for(auto v: r20i) {
            cout << " ";
            auto n = apply(CT, f, {v,f,v});
            if(n == nID) cout << " I";
            else cout << n;
        }
        cout << "\n";
    }
    cout << "\n";

    cout << "-------------------------------------------------------\n\n";
}
