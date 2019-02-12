/// \file RelKin.hh Relativistic kinematics

#ifndef RELKIN_HH
#define RELKIN_HH

#include <cmath>

// E = KE + m = gamma * m; KE = (gamma - 1)*m
// beta * gamma = sqrt(KE * (KE + 2*m))/m
// KE = sqrt(p^2 + m^2) - m
// p = m*beta*gamma = sqrt(KE*(KE + 2*m))
// 1 + beta^2 gamma^2 = gamma^2
// E^2 = p^2 + m^2

/// boost parameter gamma from velocity beta*c
inline double b2gamma(double beta) { return 1/sqrt(1-beta*beta); }
/// velocity/c from boost gamma
inline double g2beta(double gamma) { return sqrt(gamma*gamma - 1)/gamma; }
/// kinetic energy to momentum^2
inline double ke2p2(double KE, double m) { return KE*(KE+2*m); }
/// kinetic energy to momentum
inline double ke2p(double KE, double m) { return sqrt(KE*(KE+2*m)); }
/// momentum^2 to kinetic energy (low-KE stabilized)
double p22ke(double p2, double m);
/// momentum to kinetic energy (low-KE stabilized)
inline double p2ke(double p, double m) { return p22ke(p*p, m); }
/// 2-body CM momentum for total KE
double p_2body(double m1, double m2, double KE);

/// 1-dimensional Lorentz boost into a frame moving at v = beta*c
class Lorentz_boost {
public:
    double gamma = 1;   ///< boost factor
    double beta = 0;    ///< (signed) velocity/c

    /// compose with another boost (in same direction); commutative
    void operator*=(const Lorentz_boost& b);
    /// compose with another boost (in opposite direction); commutative
    void operator/=(const Lorentz_boost& b);
    /// composed boosts
    Lorentz_boost operator*(const Lorentz_boost& b) const { auto k = *this; k *= b; return k; }
    /// composed boosts
    Lorentz_boost operator/(const Lorentz_boost& b) const { auto k = *this; k /= b; return k; }

    /// boost 4-vector (v0, v1, ?, ?) in (1,0,0) direction; return boosted v0
    double boost(double v0, double& v1) const;
    /// boost 4-vector (v0, v1, ?, ?) in (-1,0,0) direction; return boosted v0
    double unboost(double v0, double& v1) const;

    /// boosted momentum component given p_|| and total p^2 (OK in p << m limit)
    double boost_p(double m, double px, double p2) const { boost(sqrt(p2+m*m), px); return px; }
    /// boosted momentum component given p_|| and total p^2 (OK in p << m limit)
    double unboost_p(double m, double px, double p2) const { unboost(sqrt(p2+m*m), px); return px; }

    /// calculate particle CM frame boost from KE, mass
    void particleCM(double KE, double m);
    /// calculate center-of-mass boost parameters given projectile KE on lab-frame static target; return KE in CM frame
    double projectileCM(double KE, double mProj, double mTarg, bool forward = true);

    /// print desciption to stdout
    void display() const;
};

/// display test calculation
void testRelKin();

#endif

