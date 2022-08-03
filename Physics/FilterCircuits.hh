/// \file FilterCircuits.hh Linear filter network circuits
// Michael P. Mendenhall, LLNL 2022

#include "ZCircuit.hh"
#include <cassert>

/// Configure N-node ladder topology circuit
/*
 * (0) -Z0- (1) -Z2- (2) .... (N-1)
 *           |        |        |
 *          Z1       Z3       Z{2N-1}
 *            \       |       /
 *             \_____Gnd_____/
*/
template<class ZC_t>
void configure_Ladder(ZC_t& C) {
    C.iOut = C.Ncalc - 1;
    C.iGnd = C.Ncalc + C.Vnodes.size();
    C.Vnodes.push_back({});     // ground node
    C.Vnodes.push_back({1});    // input voltage
    C.iV0 = C.iGnd + 1;         // input

    for(typename ZC_t::nodeidx_t i = 1; i < C.Ncalc; ++i) {
        C.addLink(i-1, i,  {});
        C.addLink(i, C.iGnd, {});
    }
    C.addLink(0, C.iV0, {});
}

/// Circuit stuffer for a Butterworth filter
template<class ZCS_t = ZCircuitStuffer<>>
class ButterworthStuffer: public ZCS_t {
public:
    using ZCS_t::ZCS_t;
    using typename ZCS_t::ZCalc_t;
protected:
    using ZCS_t::ps;
    vector<C_ZCalc<ZCalc_t>> Cs;    ///< capacitors
    vector<L_ZCalc<ZCalc_t>> Ls;    ///< inductors
    R_ZCalc<ZCalc_t> Rterm;         ///< termination resistors

public:
    /// Setup stuffing for N-node ladder filter
    void configure(size_t N) {
        ps.clear();
        Cs.clear();
        Ls.clear();

        Rterm = {1};

        int k = 1;
        for(size_t i=0; i<N; ++i) {
            Ls.emplace_back(2*sin((2*k-1)*M_PI_2/N));
            ++k;
            Cs.emplace_back(2*sin((2*k-1)*M_PI_2/N));
            ++k;
        }

        for(size_t i=0; i<N; ++i) {
            ps.emplace_back(&Ls[i]);
            ps.back().links.push_back(2*i);
            ps.emplace_back(&Cs[i]);
            ps.back().links.push_back(2*i + 1);
        }
        ps.emplace_back(&Rterm);
        ps.back().links.push_back(2*N);
    }
};
