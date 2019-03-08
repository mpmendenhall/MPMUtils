/// \file Icosahedral.cc

#include "Icosahedral.hh"
#include "FiniteGroup.hh"
#include <algorithm>

namespace Icosahedral {

    const SurdSum phi = (SurdSum::sqrt(5)+1)/2;
    const SurdSum ihp = phi.inverse();



    //const Icosahedral::vec_t Icosahedral::u12 = {{{ -phi/2,  ihp/2,    {{1,2}} }}}; // TODO 31 points?
    const vec_t u12 = {{{ phi+2,  {0},  ihp-2-(phi*2) }}}; // TODO
    const vec_t u20 = {{{ SurdSum::sqrt({1,3}), SurdSum::sqrt({1,3}), SurdSum::sqrt({1,3}) }}};
    const vec_t u30 = {{{ {1}, {0}, {0} }}};

    const elem_t R10{{{-phi/2,   ihp/2,    {{1,2}},
                        ihp/2,   {{-1,2}}, phi/2,
                        {{1,2}}, phi/2,    ihp/2 }}};

    const elem_t R58{{{ phi/2,   ihp/2,    {{-1,2}},
                        ihp/2,   {{1,2}},  phi/2,
                        {{1,2}}, -phi/2,   ihp/2 }}};

    const vector<elem_t> Rs = span<ApplyMul<elem_t>>({R10,R58});

    vector<vec_t> points(const vec_t& v) {
        vector<vec_t> vv(Rs.size());
        auto it = vv.begin();
        for(auto& M: Rs) *(it++) = M*v;
        std::sort(vv.begin(), vv.end());
        vv.erase(std::unique(vv.begin(), vv.end()), vv.end());
        return vv;
    }

    vec_t axis(const elem_t& M) {
        auto t = M.trace();
        if(t==SurdSum(-1) || t==SurdSum(3)) return {};
        return vec_t{{M(2,1)-M(1,2), M(0,2)-M(2,0), M(1,0)-M(0,1)}};
    }
}
