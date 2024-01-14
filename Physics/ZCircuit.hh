/// @file ZCircuit.hh Network of linear 2-terminal devices
// Michael P. Mendenhall, LLNL 2022

#include "Matrix.hh"
#include <vector>
using std::vector;
#include <array>
using std::array;

/// Circuit network base class
template<typename _val_t = std::complex<double>>
class ZCircuit_Base {
public:
    typedef _val_t val_t;       ///< calculation (complex, maybe symbolic) value type
    typedef size_t nodeidx_t;   ///< node identifier index

    nodeidx_t Ncalc = {};       ///< number of internal "free" calculated nodes

    /// linear link between nodes in circuit
    struct link_t {
        nodeidx_t i0;       ///< device start terminal
        nodeidx_t i1;       ///< device end terminal
        val_t Z;            ///< impedance of device
        val_t phase;        ///< delay phase shift factor for current reaching other side
    };
    vector<link_t> links;   ///< links between nodes
    vector<val_t> Vnodes;   ///< additional constrained voltage points, indexed Ncalc + i

    // common useful nodes, construction dependent interpretation
    nodeidx_t iV0  = {};    ///< "input" node index
    nodeidx_t iOut = {};    ///< "output" node index
    nodeidx_t iGnd = {};    ///< "ground" node index

    /// Helper to add link
    void addLink(nodeidx_t i0, nodeidx_t i1, val_t Z = {}, val_t phase = val_t{1}) {
        if(i0 > i1) std::swap(i0, i1);
        if(i1 > Ncalc + Vnodes.size()) throw std::logic_error("Link to invalid node number");
        links.push_back({i0, i1, Z, phase});
    }

    /// Solve circuit; return output node value
    virtual val_t solve() = 0;
};


/// output representation
template<typename val_t>
std::ostream& operator<<(std::ostream& o, const ZCircuit_Base<val_t>& C) {
    o << "ZCircuit [" << C.Ncalc << " free nodes; input " << C.iV0 << ", output " << C.iOut << ", ground " << C.iGnd << "]" << "\n";
    size_t i = C.Ncalc;
    for(const auto& v: C.Vnodes) o << "\t+ Constraint [" << i++ << "] V = " << v << "\n";
    i = 0;
    for(auto& l: C.links)  o << "\t" << "* Link [" << i++ << "]: " << l.i0 << " -> " << l.Z << " " << std::arg(l.phase) << " -> " << l.i1 << "\n";
    return o;
}

/// Network of linear 2-terminal devices with N free nodes
template<size_t N, typename _val_t = std::complex<double>>
class ZCircuit: virtual public ZCircuit_Base<_val_t> {
public:
    typedef ZCircuit_Base<_val_t> ZC_t;
    using typename ZC_t::val_t;
    using typename ZC_t::nodeidx_t;
    using ZC_t::links;
    using ZC_t::Vnodes;
    using ZC_t::iOut;
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
    ZCircuit() { ZC_t::Ncalc = N; }

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
        for(const auto& l: links) {
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
            if(c == val_t{}) continue;      // open circuit

            M(i0, i0) += c;
            M(i0, i1) -= c * l.phase;

            M(i1, i1) += c;
            M(i1, i0) -= c * l.phase;
        }

        // connect together mutually-shorted nodes; clear Vshorted rows
        for(nodeidx_t i = {}; i < N; ++i) {
            if(Vshorted[i]) {
                for(nodeidx_t j = {}; j < N; ++j) M(i, j) = val_t{};
                continue;
            }
            if(shorted[i] == i) continue;
            // TODO shorting phases
            M(i, i)          += val_t{1};
            M(i, shorted[i]) += val_t{-1};
        }

        // link-to-Vnodes terms
        for(const auto& l: links) {
            if(l.i0 >= N || l.i1 < N) continue;
            auto i0 = shorted[l.i0];
            if(l.Z == val_t{}) M(i0, i0) += l.phase;
            else if(!Vshorted[i0]) M(i0, i0) += l.phase/l.Z;
        }
    }

    /// Fill circuit equation RHS vector
    void build_RHS() {
        RHS = {};

        // External Vnode link terms
        for(const auto& l: links) {
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

    /// Build and solve, returning at output node
    val_t solve() override {
        build_M();
        solve_M();
        build_RHS();
        V = Mi*RHS;
        return iOut < N? V[iOut] : val_t{};
    }
};


/// output representation
template<size_t N, typename val_t>
std::ostream& operator<<(std::ostream& o, const ZCircuit<N, val_t>& C) {
    o << (ZCircuit_Base<val_t>&)C;
    size_t i = 0;
    for(; i < N; ++i) {
        if(C.shorted[i] > i) o << "\t* Shorted " << i << " -> " << C.shorted[i] << "\n";
        if(C.Isrc[i] != val_t{})  o << "\t* I_in " << i << " = " << C.Isrc[i] << "\n";
    }
    return o;
}


#include <complex>

/// complex magnitudes comparison needed by Matrix<std::complex<double>>
template<>
bool mag_lt<std::complex<double>>(const std::complex<double>& a, const std::complex<double>& b) { return std::norm(a) < std::norm(b); }


//-------------------------------------//
//-------------------------------------//
//-------------------------------------//


/// Circuit element calculating Z(s = sigma + i*omega)
template<typename _val_t = std::complex<double>, typename _x_t = double>
class ZCalc {
public:
    typedef _val_t val_t;
    typedef _x_t x_t;

    /// Constructor
    explicit ZCalc(x_t _R = {}, x_t _d = {}): R(_R), delay(_d) { }
    x_t R = {};      ///< constant impedance
    x_t delay = {};  ///< delay time

    /// Delay phase
    val_t phase(val_t s) const { return std::exp(-delay*s); }
    /// Complex impedance at specified s = sigma + i*omega
    val_t Z(val_t s) const { return R + _Z(s); }
    /// Complex impedance at specified s = sigma + i*omega
    virtual val_t _Z(val_t s) const = 0;
};

/// Resistor
template<class ZCalc_t = ZCalc<>>
class R_ZCalc: public ZCalc_t {
public:
    using typename ZCalc_t::x_t;
    using typename ZCalc_t::val_t;

    /// Constructor
    explicit R_ZCalc(x_t _R) { ZCalc_t::R = _R; }
    /// (Complex) impedance at specified angular frequency
    val_t _Z(val_t) const override { return val_t{}; }
};

/// Capacitor
template<class ZCalc_t = ZCalc<>>
class C_ZCalc: public ZCalc_t {
public:
    using typename ZCalc_t::x_t;
    using typename ZCalc_t::val_t;

    /// Constructor
    explicit C_ZCalc(x_t _C): C(_C) { }
    x_t C;  ///< Capacitance
    /// (Complex) impedance at specified angular frequency
    val_t _Z(val_t s) const override { return val_t{1}/(C*s); }
};

/// Inductor
template<class ZCalc_t = ZCalc<>>
class L_ZCalc: public ZCalc_t{
public:
    using typename ZCalc_t::x_t;
    using typename ZCalc_t::val_t;

    /// Constructor
    explicit L_ZCalc(x_t _L): L(_L) { }
    x_t L;  ///< Resistance
    /// (Complex) impedance at specified angular frequency
    val_t _Z(val_t s) const override { return L*s; }
};

/// "Circuit stuffer" to update Z values for circuit links
template<class _ZCalc_t = ZCalc<>>
class ZCircuitStuffer {
public:
    typedef _ZCalc_t ZCalc_t;
    typedef typename ZCalc_t::val_t val_t;
    typedef typename ZCalc_t::x_t x_t;

    /// placed component specifications
    struct s_placement {
        /// Constructor
        explicit s_placement(ZCalc_t* _C = nullptr): C(_C) { }
        ZCalc_t* C;             ///< component calculator
        val_t Z = {};           ///< latest calculated Z
        val_t phase = {};       ///< latest calculated delay phase
        vector<size_t> links;   ///< link placements for component
    };
    vector<s_placement> ps;     ///< component placements

    /// calculate frequency-dependent components Z
    void setFreq(x_t omega) { setS({{}, omega}); }
    /// calculate Laplace-plane-dependent components Z
    void setS(val_t s) {
        for(auto& p: ps) {
            if(p.C) { p.Z = p.C->Z(s); p.phase = p.C->phase(s); }
            else {    p.Z = val_t{};   p.phase = val_t{1}; }
        }
    }
    /// set Z values in circuit
    template<class ZCircuit_t>
    void stuff(ZCircuit_t& ZC) const {
        for(const auto& p: ps) {
            for(auto l: p.links) {
                ZC.links.at(l).Z = p.Z;
                ZC.links.at(l).phase = p.phase;
            }
        }
    }
};

/// Circuit evaluation function wrapper
template<class Stuffer_t = ZCircuitStuffer<>, class Circuit_t = ZCircuit_Base<>>
class CircuitEvaluator {
public:
    typedef typename Circuit_t::val_t val_t;

    /// Constructor
    CircuitEvaluator(Stuffer_t& _S, Circuit_t& _C): S(_S), C(_C) { }

    /// Evaluate response at point
    val_t operator()(val_t s) {
        S.setS(s);
        S.stuff(C);
        return C.solve();
    }

    Stuffer_t& S;   ///< (frequency-dependent) circuit stuffing instructions
    Circuit_t& C;   ///< base circuit topology
};
