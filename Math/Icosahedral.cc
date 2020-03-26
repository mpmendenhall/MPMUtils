/// \file Icosahedral.cc
// -- Michael P. Mendenhall, 2019
/*
#include "Icosahedral.hh"
#include <algorithm>
#include <iostream>
using std::cout;

namespace Icosahedral {

    const PhiField phi{0,1};
    const auto ihp = phi.inverse();
    const PhiField half{{1,2},0};
    const PhiField one{1,0};
    const PhiField zero{0,0};

    const elem_t Ra{{-phi/2,   ihp/2,    half,
                      ihp/2,   -half,    phi/2,
                      half,    phi/2,    ihp/2 }};

    const elem_t Rb{{ phi/2,   ihp/2,   -half,
                      ihp/2,   half,    phi/2,
                      half,   -phi/2,   ihp/2 }};

    const elem_t Rc{{ -one,  zero, zero,
                       zero, -one, zero,
                       zero, zero, -one }};

    const GeneratorsConjugacy<groupop_t> IC({Ra,Rb,Rc});
    const genspan_t Rs = IC.Rs;
    const cayley_t CT = IC.CT;
    const conjugacy_t CD = IC.CD;

    s_parity::s_parity() {
        auto it = begin();
        for(auto& e: Rs) *(it++) = det(e) > 0;
    }
    const s_parity parity;

    indexel_t::indexel_t(size_t ii): i(ii), o(Rs.element(i)) { }

    template<typename T>
    faceset_t<T>::faceset_t(size_t cnum) {
        // generators conjugacy group: rotation element around each face
        auto& rfi = CD.M.find(T::order())->second.CCs.getClassNum(cnum);
        if(rfi.size() != T::order()) abort();
        printf("Calculating %zu faces...\n", T::order());

        size_t n = 0;       // face number
        for(auto i: rfi) {  // rotation around face n
            auto& f = this->at(n);
            f.c = R3axis(Rs.element(i));            // face center axis
            f.g[0] = indexel_t(Nav.domain(f.c));    // from fundamental domain to one domain in face
            f.R[0] = indexel_t(nID);                // ID element

            for(size_t j=1; j<T::order(); ++j) { // successive rotations
                f.R[j] = indexel_t(CT.apply(i, f.R[j-1].i));
                f.g[j] = indexel_t(CT.apply(i, f.g[j-1].i));
            }

            size_t j = 0;
            for(auto& e: f.g) elenum[e.i] = T::order()*n + (j++);

            ++n;
        }
    }

    const faceset_t<f12_t> dodFaces(1);
    const faceset_t<f15_t> flipAxes(1);
    const faceset_t<f20_t> icoFaces(0);

    /// arbitrary point selecting representative ``fundamental'' domain
    const axis_t fd_p0{{half, half, half*20}};

    Navigator::Navigator():
    DecisionTree(n_elements, 15, [](size_t i, size_t j){ return axpart(Rs.element(i) * fd_p0, j); }) { }
    const Navigator Nav;

    template<typename F>
    const F& selectFundamental(const faceset_t<F>& a) {
        for(auto& f: a) {
            axis_t c = f.c;
            Nav.map_d0(c);
            if(f.c == c) return f;
        }
        throw std::logic_error("No supplied point in fundamental domain!");
    }
    const f12_t Navigator::v12 = selectFundamental(dodFaces);
    const f15_t Navigator::v15 = selectFundamental(flipAxes);
    const f20_t Navigator::v20 = selectFundamental(icoFaces);
}

void Icosahedral::describe() {
    cout << "\n---------------- Icosahedral Symmetry ----------------\n\n";

    cout << "Starting from two rotation and one inversion generator matrices:\n\n" << Ra << "\n" << Rb << "\n" << Rc << "\n";
    cout << "(where φ = (1+√5)/2 is the `golden ratio')\n";
    cout << "we build the Full Icosahedral symmetry point group,\n";
    CD.display();
    cout << "\n";

    cout << "The element of order 1 (#" << nID << ") is the identity transformation:\n" << Rs.element(nID) << "\n";

    auto pID = *CD.M.find(2)->second.CCs.getClassNum(0).begin();
    cout << "The single element of order 2 (#" << pID << ") is the mirror inversion:\n" << Rs.element(pID) << "\n";

    cout << "The 2x15 elements of order 2 are flips by pi (with and without inversion)\n";
    cout << "around axes through the midpoints of opposite icosahedral/dodecahedral edges:\n";
    auto& r15i = CD.M.find(2)->second.CCs.getClassNum(1);
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

    cout << "Elements of orders 6 and 10 combine parity inversion with the order-3 and order-5 rotations.\n\n";

    cout << "A point can be classified into one of 120 domains covering the sphere\n";
    cout << "using a decision tree based on direction relative to flip axes:\n";
    Nav.display();
    cout << "\n";

    cout << "We can choose one (arbitrary) representative ``fundamental domain,''\n";
    cout << "into which any point can be mapped, bounded by the triangle:\n";
    cout << Navigator::v12.c << " (dodecahedral face center)\n";
    cout << Navigator::v15.c << " (edge center)\n";
    cout << Navigator::v20.c << " (icosahedral face center)\n";
    cout << "\n";

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
*/
