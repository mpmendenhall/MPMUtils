/// @file RelKin.hh Relativistic kinematics, with numerically stable calculations in nonrelativistic limits
// -- Michael P. Mendenhall, 2020

#ifndef RELKIN_HH
#define RELKIN_HH

#include <cmath>

// gamma = 1/sqrt(1 - beta^2)
// E = KE + m = gamma * m; KE = (gamma - 1)*m
// beta * gamma = sqrt(KE * (KE + 2*m))/m
// KE = sqrt(p^2 + m^2) - m
// p = m*beta*gamma = sqrt(KE*(KE + 2*m))
// 1 + beta^2 gamma^2 = gamma^2
// E^2 = p^2 + m^2

/* Numerical stability notes.
 *
 * KE = sqrt(p2 + m*m) - m = m*(sqrt(1 + (p/m)^2) - 1)
 * but we want stable evaluation in p << m classical limit.
 * Let x = (p/m)^2: sqrt(1 + x) - 1 = exp(0.5 * ln(1 + x)) - 1
 * use cmath log1p, expm1 numerically stable implementations
 *
 * gM1 = gamma - 1 is numerically better than gamma
 * gM1 = 1/sqrt(1 - beta^2) - 1 = exp(-0.5*ln(1 - x^2))
 * beta = sqrt(gamma^2 - 1)/gamma = sqrt(gM1^2 + 2*gM1)/(1 + gM1)
 */

/// boost parameter gamma from velocity beta*c
inline double beta_to_gamma(double beta) { return 1/sqrt(1 - beta*beta); }
/// boost parameter gamma - 1 from velocity beta*c
inline double beta_to_gammaM1(double beta) { return expm1(-0.5*log1p(-beta*beta)); }

/// velocity/c from boost gamma
inline double gamma_to_beta(double gamma) { return sqrt(gamma*gamma - 1)/gamma; }
/// velocity/c from boost gamma - 1
inline double gammaM1_to_beta(double gammaM1) { return sqrt(gammaM1*gammaM1 + 2*gammaM1)/(1 + gammaM1); }

/// kinetic energy to momentum^2
inline double KE_to_p2(double KE, double m) { return KE*(KE+2*m); }
/// kinetic energy to momentum
inline double KE_to_p(double KE, double m) { return sqrt(KE_to_p2(KE,m)); }
/// momentum^2 to kinetic energy
inline double p2_to_KE(double p2, double m) { return m? m*expm1(0.5*log1p(p2/(m*m))) : sqrt(p2); }
/// momentum to kinetic energy
inline double p_to_KE(double p, double m) { return m? p2_to_KE(p*p, m) : p; }

/// 2-body CM momentum (equal and opposite for each), given sum KE
double p_2body(double m1, double m2, double KE);

/// 1-dimensional Lorentz boost into a frame moving at v = beta*c
class Lorentz_boost {
public:
    //  g   -bg
    //    1
    //      1
    // -bg    g

    double gammaM1 = 0; ///< boost factor gamma - 1
    double beta = 0;    ///< (signed) velocity/c

    /// get gamma
    inline double gamma() const { return gammaM1 + 1; }

    //----- define boost -----//

    /// Default identity boost constructor
    Lorentz_boost() { }
    /// construct from beta
    static Lorentz_boost from_beta(double b) { return Lorentz_boost(beta_to_gammaM1(b), b); }
    /// construct from gamma - 1
    static Lorentz_boost from_gammaM1(double gm1) { return Lorentz_boost(gm1, gammaM1_to_beta(gm1)); }

    /// inverse boost
    Lorentz_boost inverse() const { return Lorentz_boost(gammaM1, -beta); }

    /// set value from beta
    void setBeta(double b) { beta = b; gammaM1 = beta_to_gammaM1(b); }
    /// set value from gamma - 1
    void setGammaM1(double gm1) { gammaM1 = gm1; beta = gammaM1_to_beta(gm1); }

    /// calculate boost into center-of-mass frame for particle with KE, mass
    static Lorentz_boost to_particle_CM(double KE, double m) { return from_gammaM1(KE/m); }
    /// calculate center-of-mass boost parameters given projectile KE on lab-frame static target; update KE to CM frame value
    static Lorentz_boost to_projectile_CM(double& KE, double mProj, double mTarg, bool forward = true);

    //----- compose boosts -----//

    /// compose with another boost (in same direction); commutative
    void operator*=(const Lorentz_boost& b);
    /// compose with another boost (in opposite direction); commutative
    void operator/=(const Lorentz_boost& b);
    /// composed boosts
    Lorentz_boost operator*(const Lorentz_boost& b) const { auto k = *this; k *= b; return k; }
    /// composed boosts
    Lorentz_boost operator/(const Lorentz_boost& b) const { auto k = *this; k /= b; return k; }

    //----- apply boost -----//

    /// boost 4-vector (v0, v1, ?, ?) in (1,0,0) direction
    void boost(double& v0, double& v1) const;
    /// boost 4-vector (v0, v1, ?, ?) in (-1,0,0) direction
    void unboost(double& v0, double& v1) const;

    /// boosted momentum component given p_|| and total p^2
    double boost_p(double m, double px, double p2) const { double E = sqrt(p2+m*m); boost(E, px); return px; }
    /// boosted momentum component given p_|| and total p^2
    double unboost_p(double m, double px, double p2) const { double E = sqrt(p2+m*m); unboost(E, px); return px; }

    /// boost massless particle unit direction component (along boost axis)
    double boost_d(double d) const { return (d - beta)/(1 - beta*d); }
    /// unboost massless particle unit direction component (along boost axis)
    double unboost_d(double d) const { return (d + beta)/(1 + beta*d); }
    /// unit direction boost Jacobean d(boost_d)/dd
    double boost_dd(double d) const { double c = 1 - beta*d; return (1-beta*beta)/(c*c); }
    /// unit direction boost Jacobean d(unboost_d)/dd
    double unboost_dd(double d) const { double c = 1 + beta*d; return (1-beta*beta)/(c*c); }

    /// given lab-frame unit direction component d_z and CM-frame energy, calculate CM frame momentum component p_z
    /// (useful in 2-body CM systems where CM-frame E,|p| is known, and lab direction is measured)
    double pz_CM_from_lab_direction(double E_CM, double m, double dz_lab) const;

    /// given lab-frame unit direction component d_z and CM-frame |p|, calculate CM frame unit direction d_z
    double dz_CM_from_lab_direction(double p_CM, double m, double dz_lab) const;

    /// print desciption to stdout
    void display() const;

protected:
    /// Constructor with gammaM1, beta
    Lorentz_boost(double gm1, double b): gammaM1(gm1), beta(b) { }
};

/// display test calculation
void testRelKin();

#endif

