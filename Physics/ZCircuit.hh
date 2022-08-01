/// \file ZCircuit.hh Network of linear 2-terminal devices
// Michael P. Mendenhall, LLNL 2022

#include "Matrix.hh"
#include <vector>
using std::vector;
#include <array>
using std::array;

/// Circuit network base class
template<typename _val_t>
class ZCircuit_Base {
public:
    typedef _val_t val_t;  ///< calculation (complex, maybe symbolic) value type
    typedef size_t nodeidx_t;   /// node identifier index

    /// linear link between nodes in circuit
    struct link_t {
        nodeidx_t i0;       ///< device start terminal
        nodeidx_t i1;       ///< device end terminal
        val_t Z;            ///< impedance of device
    };
    vector<link_t> links;   ///< links between nodes
    vector<val_t> Vnodes;   ///< additional constrained voltage points, indexed N + i

    /// Helper to add link
    void addLink(nodeidx_t i0, nodeidx_t i1, val_t Z = {}) {
        if(i0 > i1) std::swap(i0, i1);
        links.push_back({i0, i1, Z});
    }
};

/// Network of linear 2-terminal devices with N free nodes
template<size_t N, typename _val_t>
class ZCircuit: virtual public ZCircuit_Base<_val_t> {
public:
    typedef ZCircuit_Base<_val_t> ZC_t;
    using typename ZC_t::val_t;
    using typename ZC_t::nodeidx_t;
    using ZC_t::links;
    using ZC_t::Vnodes;
    static constexpr size_t NNodes = N; ///< number of free nodes
    typedef Matrix<N,N,val_t> Mat_t;    ///< circuit equations matrix
    typedef Matrix<N,1,val_t> Vec_t;    ///< circuit equations RHS vector

    array<nodeidx_t, N> shorted = {};   ///< highest internal node to which each node is shorted
    array<bool, N> Vshorted = {};       ///< whether (shorting group) is shorted to Vnode
    array<val_t, N> Isrc = {};          ///< current source (+) or sink (-) term attached to each node

    Mat_t M;                ///< circuit equation matrix M * V = RHS
    Vec_t RHS;              ///< circuit equation M*V = RHS vector
    Mat_t Mi;               ///< circuit solution M^-1: V = Mi * RHS
    Vec_t V;                ///< solution voltages at free nodes

    /// Constructor
    ZCircuit() {
        for(nodeidx_t i = {}; i < N; ++i) shorted[i] = i;
    }

    /// Fill circuit matrix
    void build_M() {
        M = {};

        // normalize orientation for easier constraint-node checks
        for(auto& l: links) if(l.i0 > l.i1) std::swap(l.i0, l.i1);

        // calculate internal shorting graph
        Vshorted = {};
        for(nodeidx_t i = {}; i < N; ++i) shorted[i] = i;
        for(auto& l: links) {
            if(l.i0 > l.i1) std::swap(l.i0, l.i1);  // normalize orientation for easier constraint-node checks
            if(l.Z != val_t{} || l.i0 >= N || l.i1 >= N || l.i0 == l.i1) continue; // not an internal short

            auto ix = shorted[l.i0];
            if(ix == l.i1) continue; // previously-identified short
            if(ix > l.i1) shorted[l.i1] = ix;
            else shorted[ix] = shorted[l.i0] = l.i1;
        }
        // follow shorted paths to common highest node
        for(nodeidx_t i = {}; i < N; ++i) {
            auto& u = shorted[i];
            while(u != shorted[u]) u = shorted[u];
        }

        // internal non-shorting links between shorting equivalence classes
        for(auto& l: links) {
            if(l.i0 >= N) continue; // irrelevant Vnode-to-Vnode link
            auto i0 = shorted[l.i0];

            if(l.Z == val_t{}) {
                if(l.i1 >= N) Vshorted[i0] = true; // tag shorts to external
                continue;
            }

            if(l.i1 >= N) continue;
            auto i1 = shorted[l.i1];

            if(i0 == i1) continue;          // link to itself (maybe via shorts)

            auto c = val_t{1}/l.Z;
            if(c == val_t{}) continue; // open circuit

            M(i0, i0) += c;
            M(i0, i1) -= c;
            M(i1, i1) += c;
            M(i1, i0) -= c;
        }

        // connect together mutually-shorted nodes; clear Vshorted rows
        for(nodeidx_t i = {}; i < N; ++i) {
            if(Vshorted[i]) {
                for(nodeidx_t j = {}; j < N; ++j) M(i, j) = val_t{};
                continue;
            }
            if(shorted[i] == i) continue;
            M(i, i)          += val_t{1};
            M(i, shorted[i]) += val_t{-1};
        }

        // link-to-Vnodes terms
        for(auto& l: links) {
            if(l.i0 >= N || l.i1 < N) continue;
            auto i0 = shorted[l.i0];
            if(l.Z == val_t{}) M(i0, i0) += val_t{1};
            else if(!Vshorted[i0]) M(i0, i0) += val_t{1}/l.Z;
        }
    }

    /// Fill circuit equation RHS vector
    void build_RHS() {
        RHS = {};

        // External Vnode link terms
        for(auto& l: links) {
            if(l.i0 >= N || l.i1 < N) continue; // links from internal to Vnode
            auto i0 = shorted[l.i0]; // common shorting point
            if(l.Z == val_t{}) RHS[i0] +=  Vnodes.at(l.i1-N);
            else if(!Vshorted[i0]) RHS[i0] +=  Vnodes.at(l.i1-N)/l.Z;
        }

        // Current source additions (if not shorted to Vnode)
        for(nodeidx_t i = {}; i < N; ++i) {
            if(Isrc[i] == val_t{}) continue;
            auto ix = shorted[i];
            if(!Vshorted[ix]) RHS[ix] += Isrc[i];
        }
    }

    /// Solve filled matrix
    void solve_M() { LUPDecomp<N,val_t>(M).inverse(Mi); }

    /// Build and solve
    void solve() {
        build_M();
        solve_M();
        build_RHS();
        V = Mi*RHS;
    }
};


/// output representation
template<size_t N, typename val_t>
std::ostream& operator<<(std::ostream& o, const ZCircuit<N, val_t>& C) {
    o << "ZCircuit [" << N << " free nodes]" << "\n";
    size_t i = 0;
    for(; i < N; ++i) {
        if(C.shorted[i] != i) o << "\t* Shorted " << i << " -> " << C.shorted[i] << "\n";
        if(C.Isrc[i] != val_t{})  o << "\t* I_in " << i << " = " << C.Isrc[i] << "\n";
    }
    for(auto& v: C.Vnodes) o << "\t[" << i++ << "] V = " << v << "\n";
    for(auto& l: C.links)  o << "\t" << "* "<< l.i0 << " -> " << l.Z << " -> " << l.i1 << "\n";
    return o;
}


#include <complex>

/// complex magnitudes comparison needed by Matrix<std::complex<double>>
template<>
bool mag_lt<std::complex<double>>(const std::complex<double>& a, const std::complex<double>& b) { return std::norm(a) < std::norm(b); }


//-------------------------------------//
//-------------------------------------//
//-------------------------------------//

/// Circuit element calculating Z(omega)
template<typename _val_t = std::complex<double>, typename x_t = double>
class Zcalc {
public:
    /// (Complex) impedance at specified angular frequency
    virtual _val_t operator()(x_t omega) const = 0;
};

/// Resistor
template<typename _val_t = std::complex<double>, typename x_t = double>
class R_Zcalc: public Zcalc<_val_t, x_t> {
public:
    /// Constructor
    explicit R_Zcalc(x_t _R): R(_R) { }
    x_t R;  ///< Resistance
    /// (Complex) impedance at specified angular frequency
    _val_t operator()(x_t) const override { return R; }
};

/// Capacitor
template<typename _val_t = std::complex<double>, typename x_t = double>
class C_Zcalc: public Zcalc<_val_t, x_t> {
public:
    /// Constructor
    explicit C_Zcalc(x_t _C, x_t _Rs = {}): C(_C), Rs(_Rs) { }
    x_t C;  ///< Capacitance
    x_t Rs; ///< series parasitic resistance
    /// (Complex) impedance at specified angular frequency
    _val_t operator()(x_t omega) const override { return {Rs, -1/(omega*C)}; }
};

/// Inductor
template<typename _val_t = std::complex<double>, typename x_t = double>
class L_Zcalc: public Zcalc<_val_t, x_t> {
public:
    /// Constructor
    explicit L_Zcalc(x_t _L, x_t _Rs = {}): L(_L), Rs(_Rs) { }
    x_t L;  ///< Resistance
    x_t Rs; ///< series parasitic resistance
    /// (Complex) impedance at specified angular frequency
    _val_t operator()(x_t omega) const override { return {Rs, omega*L}; }
};

//-------------------------------------//
//-------------------------------------//
//-------------------------------------//


/// L/R/C chained filter base class interface
template<typename _val_t = std::complex<double>>
class LRCFilterBase: virtual public ZCircuit_Base<_val_t> {
public:
    typedef ZCircuit_Base<_val_t> ZC_t;
    using typename ZC_t::nodeidx_t;

    nodeidx_t iGnd = {};
    nodeidx_t iV0 = {};
    nodeidx_t iIn = {};
    nodeidx_t iOut = {};

    /// Set component values and solve, returning output voltage
    virtual _val_t setZ(_val_t Z1, _val_t Z2) = 0;
};

/// N-pole L/R/C chained filter
/*
 * Vin                        Vout
 * (0) -Z1- (1) -Z1- (2) .... (N)
 *           |        |        |
 *           Z2       Z2       Z2
 *            \       |       /
 *             \_____Gnd_____/
*/
template<size_t N, typename _val_t = std::complex<double>>
class LRCFilter: public LRCFilterBase<_val_t>, public ZCircuit<N+1, _val_t> {
public:
    typedef LRCFilterBase<_val_t> LF_t;
    using LF_t::iGnd;
    using LF_t::iV0;
    using LF_t::iIn;
    using LF_t::iOut;
    typedef ZCircuit<N+1, _val_t> ZC_t;
    using typename ZC_t::val_t;
    using ZC_t::Vnodes;
    using ZC_t::V;
    using ZC_t::links;
    using ZC_t::addLink;
    using ZC_t::solve;


    /// Constructor
    explicit LRCFilter(bool reverse = false, bool isrc = false) {
        iGnd = N + 1;
        iV0  = N + 2;
        iIn = {};
        iOut = N;

        Vnodes.emplace_back();   // ground, index N + 1
        Vnodes.emplace_back(1);  // input voltage, index N + 2
        for(size_t i=0; i<N; ++i) {
            addLink(i, i+1);     // to next stage
            addLink(i+1, iGnd);  // to ground
        }
        if(reverse) std::swap(iIn, iOut);
        if(isrc) ZC_t::Isrc[iIn] = val_t{1};
        else addLink(iIn, iV0, {});
    }

    /// Set component values and solve, returning output voltage
    val_t setZ(val_t Z1, val_t Z2) override {
        for(size_t i=0; i<N; ++i) {
            links.at(2*i).Z = Z1;
            links.at(2*i+1).Z = Z2;
        }
        solve();
        return V[iOut];
    }
};
