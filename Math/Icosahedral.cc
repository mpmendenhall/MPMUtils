/// \file Icosahedral.cc

#include "Icosahedral.hh"
#include "FiniteGroup.hh"
#include <algorithm>

const SurdSum phi = (SurdSum::sqrt(5)+1)/2;
const SurdSum ihp = phi.inverse();

const IcosahedralSymmetry::elem_t R10{{{-phi/2,  ihp/2,    {{1,2}},
                                        ihp/2,   {{-1,2}}, phi/2,
                                        {{1,2}}, phi/2,    ihp/2 }}};

const IcosahedralSymmetry::elem_t R58{{{phi/2,   ihp/2,    {{-1,2}},
                                        ihp/2,   {{1,2}},  phi/2,
                                        {{1,2}}, -phi/2,   ihp/2 }}};

const vector<IcosahedralSymmetry::elem_t> IcosahedralSymmetry::Rs = span<ApplyMul<IcosahedralSymmetry::elem_t>>({R10,R58});

vector<IcosahedralSymmetry::vec_t> IcosahedralSymmetry::points(const vec_t& v) {
    vector<vec_t> vv(Rs.size());
    auto it = vv.begin();
    for(auto& M: Rs) *(it++) = M*v;
    std::sort(vv.begin(), vv.end());
    vv.erase(std::unique(vv.begin(), vv.end()), vv.end());
    return vv;
}
