/// @file PoleFinder.hh Numerically find poles/zeros of complex rational functions
// Michael P. Mendenhall, LLNL 2022

#include <vector>
using std::vector;
#include <complex>
#include <cmath>
#include <set>
using std::set;

/// evaluation point info
template<typename _val_t = std::complex<double>>
struct eval_t {
    typedef _val_t val_t;

    /// default constructor
    eval_t(): s(val_t{}), F(val_t{}) { }
    /// initialize from function
    template<class F_t>
    eval_t(F_t& _F, val_t _s): s(_s), F(_F(s)) { }

    val_t s;    ///< evaluation point
    val_t F;    ///< value F(s)
};

/// Candidate pole or zero
template<typename _val_t = std::complex<double>>
struct pole_t {
    typedef _val_t val_t;

    /// Constructor
    pole_t(val_t _s0, int _mult):
    s0(_s0), isReal(s0.imag() == 0), mult(_mult) { }

    val_t s0;               ///< center
    bool isReal;            ///< restrict to real axis
    int mult = 1;           ///< multiplicity exponent; positive for zeros, negative for poles
    eval_t<val_t> e1;       ///< previous evaluation point and value
    eval_t<val_t> e2;       ///< previous evaluation point and value

    /// take one step closer to pole
    template<class F_t>
    void step_closer(F_t& F) {
        e2 = e1;
        val_t s3 = (s0 + e1.s)/val_t{2};
        e1 = eval_t<val_t>(F, s3);
    }

    /// update estimates for s0 using survey points
    auto update_estimate() {
        val_t s1 = {};

        if(mult == -1) {
            auto u = e1.F - e2.F;
            s1 = (e1.s * e1.F - e2.s * e2.F)/u;
        } else if(mult == 1) {
            auto u = e1.F*e2.s - e2.F*e1.s;
            s1 = u/(e1.F - e2.F);
        } else {
            auto u = std::pow(e1.F / e2.F, 0.5/mult);
            s1 = (e1.s / u - e2.s * u)/(val_t{1}/u - u);
        }

        std::swap(s0,s1);
        if(isReal) s0 = s0.real();
        return std::norm(s1-s0)/std::norm(e1.s - e2.s);
    }

    /// quality check should match multiplicity exponent
    auto check_quality() const {
        return log(std::norm(e2.F)/std::norm(e1.F)) / log(std::norm(s0 - e2.s)/std::norm(s0 - e1.s));
    }

    /// re-evaluate at current points
    template<class F_t>
    auto refine(F_t& F) {
        if(!isReal) {
            e2 = {F, {.98*s0.real(), 0.96 * s0.imag()}};
            e1 = {F, {.99*s0.real(), 0.98 * s0.imag()}};
        } else {
            e2 = {F, 0.98 * s0};
            e1 = {F, 0.99 * s0};
        }
        return update_estimate();
    }

};

/// Pole finder for function class F_t implementing val_t F_t(val_t)
template<class _pole_t = pole_t<>>
class PoleFinder {
public:
    /// Pole description type
    typedef _pole_t pole_t;
    /// Function type being evaluated
    typedef typename pole_t::val_t val_t;
    /// mag^2 norm type
    typedef decltype(std::norm(val_t{})) norm_t;

    vector<eval_t<val_t>> testgrid; ///< points evaluated on test grid
    set<size_t> checkstart;         ///< points already checked as candidate poles
    vector<pole_t> poles;           ///< candidate poles and zeros
    val_t F0 = {1};                 ///< overall normalization
    int verbose = 1;                ///< printout verbosity

    /// fill evaluation test points grid
    template<class F_t>
    void scan_grid(F_t& F, val_t s0, val_t s1, size_t ns = 10, size_t nw = 0) {
        if(!nw) nw = ns;
        for(size_t si = 0; si < ns; ++si) {
            auto ss = (si*s0.real() + (ns-si-1)*s1.real())/(ns-1);
            for(size_t wi = 0; wi < nw; wi++) testgrid.emplace_back(F, val_t{ss, (wi*s0.imag() + (nw-wi-1)*s1.imag())/(nw-1)});
        }
    }

    /// combined poles, zeros product at position, up to overall normalization factor
    val_t operator()(val_t s, size_t allbut = 9999) const {
        val_t F = F0;
        size_t i = 0;
        for(auto& p: poles) {
            if(!p.isReal) { // conjugate for all non-real-axis poles
                for(int m = 0; m < p.mult; ++m) F *= s - std::conj(p.s0);
                for(int m = p.mult; m < 0; ++m) F /= s - std::conj(p.s0);
            }
            if(i++ == allbut) continue;
            for(int m = 0; m < p.mult; ++m) F *= s - p.s0;
            for(int m = p.mult; m < 0; ++m) F /= s - p.s0;
        }
        return F;
    }

    /// information on excursions from model
    struct minmax_excursion {
        size_t imin = {};       ///< index of minimum discrepancy
        size_t imax = {};       ///< index of maximum discrepancy
        norm_t umax = {};       ///< maximum discrepancy mag^2
        norm_t umin = 1/umax;   ///< minimum discrepancy mag^2
        norm_t umed = {};       ///< median discrepancy mag^2
    };

    /// find highest, lowest most discrepant point on testgrid
    minmax_excursion testgrid_minmax_index() const {
        minmax_excursion mm = {};
        if(!testgrid.size()) return mm;
        size_t i = 0;
        vector<norm_t> vnorms;
        for(auto& p: testgrid) {
            auto u = std::norm(p.F / (*this)(p.s));
            vnorms.push_back(u);
            if(u < mm.umin) { mm.umin = u; mm.imin = i; }
            if(u > mm.umax) { mm.umax = u; mm.imax = i; }
            ++i;
        }
        std::sort(vnorms.begin(), vnorms.end());
        mm.umed = vnorms[vnorms.size()/2];
        return mm;
    }

    /// Wrapper to modify function by dividing out known poles
    template<class F_t>
    struct polefunc_t {
        /// Constructor
        polefunc_t(F_t& _F, PoleFinder& _PF, size_t _i = 9999): F(_F), PF(_PF), i(_i) { }

        F_t& F;         ///< function to evaluate
        PoleFinder& PF; ///< other poles to exclude,
        size_t i;       ///< except pole i

        /// evaluate except for pole i
        val_t operator()(val_t s) { return F(s)/PF(s,i); }
    };

    /// Walk in candidate pole
    template<class F_t>
    void add_pole(F_t& F, pole_t p) {
        polefunc_t<F_t> PF(F, *this, poles.size());
        p.e1 = {PF, 0.80 * p.s0};
        p.e2 = {PF, 0.90 * p.s0};

        poles.push_back(p);
        norm_t u = 0;
        do {
            u = p.update_estimate();
            if(verbose) std::cout << u << "\t" << p;
            p.step_closer(PF);
        } while(u > 1e-3);

        while(refine_poles(F) > 1e-4) { }
    }

    /// Identify and refine next candidate pole/zero; return whether new pole added
    template<class F_t>
    bool find_new_pole(F_t& F) {
        auto mm = testgrid_minmax_index();
        auto r = mm.umax/mm.umin;
        if(verbose) std::cout << "\n Test grid excursion " << r - 1;
        if(r < 1.5) {
            if(verbose) std::cout << " is already good.\n";
            return false;
        }

        // check for pole or zero?
        bool ispole = mm.umax / mm.umed > mm.umed/mm.umin;

        auto i0 = ispole? mm.imax : mm.imin;
        if(checkstart.count(i0)) return false; // already tried this one!
        checkstart.insert(i0);

        auto& p0 = testgrid[i0];
        pole_t p = {p0.s, ispole? -1 : 1};


        //if(ispole) p.isReal = false;
        if(p.isReal) p.mult = 2;
        if(verbose) std::cout << "; aiming for point " << p0 << "\n\n";
        add_pole(F, p);
        return true;
    }

    /// Characterize F by adding poles to model until converged
    template<class F_t>
    void fit(F_t& F) {
        while(find_new_pole(F)) { }
        int i = 5;
        while(--i && refine_poles(F) > 1e-6) { }
        setF0(F({}));

        if(verbose) {
            auto mm = testgrid_minmax_index();
            std::cout << "\nFinal test grid excursions " << mm.umin - 1 << " to " << mm.umax - 1 << "\n\n";
        }
    }

    /// One pass of pole position refinement; return maximum change
    template<class F_t>
    norm_t refine_poles(F_t& F) {
        if(verbose) std::cout << "\nRefining...\n";
        polefunc_t<F_t> PF(F, *this, 0);
        norm_t nmax = {};
        for(auto& p: poles) {
            auto d = p.refine(PF);
            if(verbose) std::cout << d << "\t" << p;
            nmax = std::max(d, nmax);
            ++PF.i;
        }
        return nmax;
    }

    /// Set overall normalization to value at point
    void setF0(val_t _F0 = val_t{1}, val_t s = {}) {
        F0 = val_t{1};
        F0 = _F0/(*this)(s);
    }
};

/// output representation of evaluated point
template<typename val_t>
std::ostream& operator<<(std::ostream& o, const eval_t<val_t>& e) { return o << "F" << e.s <<"\t= " << e.F; }
/// output representation of pole candidate
template<typename val_t>
std::ostream& operator<<(std::ostream& o, const pole_t<val_t>& p) {
    o << (p.mult < 0? "Pole" : "Zero");
    if(abs(p.mult) != 1) o << "^" << p.mult;
    return o << "\ts0 = " << p.s0 << ":\t" << p.e1.F << " @ " << (p.e1.s - p.s0) << ",\t" << p.e2.F << " @ " << (p.e1.s - p.s0) << "\t" << "\n";
}
