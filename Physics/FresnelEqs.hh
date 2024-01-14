/// @file FresnelEqs.hh Fresnel Equations for refraction/reflection at index mismatch
// Michael P. Mendenhall, LLNL 2021

#ifndef FRESNELEQS_HH
#define FRESNELEQS_HH

#include <cmath>
#include <vector>
using std::vector;
#include <stdio.h>
#include <utility>
using std::pair;

/// TIR critical angle moving from n1 to n2 (r = n1/n2 > 1)
inline double cth_TIR(double r) { return sqrt(1 - 1/(r*r)); }

/// TIR critical angle moving from n1 to n2 (ri = n2/n1 < 1)
inline double cth_TIR_i(double ri) { return sqrt(1 - ri*ri); }

/// transmitted cos theta as function of incident cos theta >= cth_TIR, moving from n1 to n2 (r = n1/n2)
inline double cth_tx(double cth_i, double r) { return sqrt(1-r*r*(1-cth_i*cth_i)); }

/// S-polarization Fresnel reflected power moving from n1 to n2 (r = n1/n2) at incident cos(theta)
inline double Fresnel_R_s(double ci, double r) {
    double ct = cth_tx(ci, r);
    double rR = (r*ci - ct)/(r*ci + ct);
    return rR*rR;
}

/// P-polarization Fresnel reflected power moving from n1 to n2 (r = n1/n2) at incident cos(theta)
inline double Fresnel_R_p(double ci, double r) {
    double ct = cth_tx(ci, r);
    double rR = (r*ct - ci)/(r*ct + ci);
    return rR*rR;
}

/// Normal-incidence (ci = ct = 1) Fresnel reflection moving from n1 to n2 (r = n1/n2)
inline double Fresnel_Rnormal(double r) {
    double rR = (r-1)/(r+1);
    return rR*rR;
}

/// cos(Brewster's Angle) => Rp = 0 moving from n1 to n2 (r = n1/n2)
inline double cth_Brewsters(double r) { return r/sqrt(1 + r*r); }

//------------------------
//------------------------

/// Index change from n1 to n2, with (internal-structure-dependent) critical angles
struct s_IndexChange {
    /// Constructor with calculation from r = n1/n2. r = 0 is a perfect mirror.
    explicit s_IndexChange(double _r = 1): r(_r),
    ccrit(r > 1? cth_TIR(r) : 0.), bcrit(r < 1? cth_TIR_i(r) : 0.) { }

    /// compose for combined critical angle and index change
    void operator+=(const s_IndexChange& X);

    /// display info to stdout
    void display(bool end_l = true) const;

    double r = 1;       ///< index mismatch n1/n2
    double ccrit = 0;   ///< TIR critical angle (forward direction); steepest backwards angle
    double bcrit = 0;   ///< TIR critical angle (backward direction); steepest forwards angle
};

//------------------------
//------------------------

/// Reflection/Transmission coefficients
struct s_RxTx {
    /// Constructor
    s_RxTx(double _Rx, double _Tx): Rx(_Rx), Tx(_Tx) { }
    /// Default Constructor
    s_RxTx() { }

    double Rx = 0;  ///< reflected power fraction
    double Tx = 1;  ///< transmitted power fraction

    /// generate reflector
    static s_RxTx mirror(double _Rx = 1) { return {_Rx, 0.}; }
    /// generate attenuator
    static s_RxTx attenuator(double _Tx = 1) { return {0., _Tx}; }
    /// set Fresnel coefficients for S: (ci,ct) or P: (ct,ci) polarizations, pre-attenuation factor A
    void setFresnel(double c0, double c1, double r, double A = 1);

    /// print to stdout
    void display(const char* end_l = "\n") const { printf("[%5.3f|%5.3f]%s", Rx, Tx, end_l); }
    /// compose with appended Rx/Tx surface
    void operator+=(const s_RxTx& X);

    /// return transmission out from source wedged between: (first) <-- S0 <-- I0 I1 --> S1 --> (second)
    static pair<double,double> between(const s_RxTx& S0, const s_RxTx& S1, double I0 = 0.5, double I1 = 0.5);
};

//------------------------
//------------------------

/// Shared calculation of Fresnel reflection coefficients and angle
struct s_Fresnel_Rx: public s_IndexChange {
    /// Constructor with calculation from incident cos theta, r = n1/n2
    explicit s_Fresnel_Rx(double _r = 1, double _ci = -1, double _A0 = 1):
    s_IndexChange(_r), A0(_A0) { if(_ci >= 0) set_ci_single(_ci); }

    /// set incident angle, assuming single-surface r
    void set_ci_single(double _ci);

    /// display info to stdout
    void display() const;

    /// compose (with matching ct -> X.ci)
    void operator+=(const s_Fresnel_Rx& X);

    double A0 = 1;  ///< pre-attenuation factor before interface, for ci = 0
    double ci = 0;  ///< incident cos theta
    double ct = 0;  ///< transmitted cos theta

    s_RxTx X_s;     ///< S polarization Rx/Tx
    s_RxTx X_p;     ///< P polarization Rx/Tx
};

//------------------------
//------------------------

/// Total from sequential index mismatches {r1, r2, ...}
class FresnelStack: public vector<s_Fresnel_Rx>, public s_Fresnel_Rx {
public:
    /// Inherit constructors
    using vector::vector;

    /// calculate for incident cos theta
    void set_cth0(double cth);

    /// print info to stdout
    void display() const;
};

#endif
