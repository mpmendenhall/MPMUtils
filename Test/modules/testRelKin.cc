/// @file testRelKin.cc Test of relativistic kinematics calcs

#include "ConfigFactory.hh"
#include "RelKin.hh"
#include "NuclPhysConstants.hh"
using namespace physconst;

#include <stdio.h>
#include <array>

/// particle information
struct ptcl_t {
    float M_amu = 0;            ///< mass [AMU]
    float A = 0;                ///< atomic mass number, n_neutrons + n_protons( = Z)
    float Z = 0;                ///< charge [e]
    float E = 0;                ///< kinetic energy [MeV]
    std::array<float,3> d = {{}};   ///< unit direction vector

    /// print debugging info
    void display() const { printf("M = %.3f AMU\tZ = %.1f\tE = %.4f MeV\n", M_amu, Z, E); }
};

/// Base for projectile/target kinematics generator
class KinematicsGenerator: protected Lorentz_boost {
public:
    double EnCM = 0;        ///< center-of-mass KE [MeV]
    double m_proj = 0;      ///< projectile mass [MeV]
    double m_targ = 0;      ///< target mass [MeV]

    /// Calculate center-of-mass kinematics from lab-frame incident energy
    virtual void setIncident(double EnLab, double cmVelSign);

    /// Boost particle from lab to CM frame, given lab-frame unit direction vector f.d
    void toCM(ptcl_t& f) const;
    /// Return ptcl_t from CM to lab frame, given CM-frame unit direction vector f.d
    void toLab(ptcl_t& f) const;
};

/// 6Li kinematics calculator
class Li6Gen: public KinematicsGenerator {
public:
    /// Constructor
    Li6Gen();

    /// Calculate center-of-mass boost from lab-frame incident energy
    void setIncident(double EnLab, double cmVelSign) override;

    const double Qn6Li  = 4.78365;  ///< Q-value for n+6Li -> a+t+Q (MeV)
    double p_CM = 0;                ///< CM momentum magnitude for each outgoing particle
    ptcl_t n6Li_a;                  ///< alpha for n + 6Li event (CM frame)
    ptcl_t n6Li_t;                  ///< triton for n + 6Li event (CM frame)
};

void KinematicsGenerator::setIncident(double EnLab, double cmVelSign) {
    EnCM = EnLab;
    (Lorentz_boost&)(*this) = to_projectile_CM(EnCM, m_proj, m_targ, cmVelSign > 0);
}

void KinematicsGenerator::toLab(ptcl_t& f) const {
    double M = f.M_amu * m_amu;
    double p = KE_to_p(f.E, M);
    for(auto& d: f.d) d *= p;

    f.d[2] = unboost_p(M, f.d[2], p*p);
    auto p2 = f.d[0]*f.d[0] + f.d[1]*f.d[1] + f.d[2]*f.d[2];
    f.E = p2_to_KE(p2, M);
    for(auto& d: f.d) d /= sqrt(p2);
}

void KinematicsGenerator::toCM(ptcl_t& f) const {
    double M = f.M_amu * m_amu;
    double p = KE_to_p(f.E, M);
    for(auto& d: f.d) d *= p;

    f.d[2] = boost_p(M, f.d[2], p*p);
    auto p2 = f.d[0]*f.d[0] + f.d[1]*f.d[1] + f.d[2]*f.d[2];
    f.E = p2_to_KE(p2, M);
    for(auto& d: f.d) d /= sqrt(p2);
}

//------------------------------------

Li6Gen::Li6Gen() {
    m_proj = m_n;
    m_targ = m_6Li;

    n6Li_t.M_amu = m_triton/m_amu;
    n6Li_t.A = 3;
    n6Li_t.Z = 1;

    n6Li_a.M_amu = m_alpha/m_amu;
    n6Li_a.A = 4;
    n6Li_a.Z = 2;

    Li6Gen::setIncident(0, 1);
}

void Li6Gen::setIncident(double EnLab, double cmVelSign) {
    KinematicsGenerator::setIncident(EnLab, cmVelSign);
    p_CM = p_2body(m_alpha, m_triton, Qn6Li + EnCM);
    n6Li_t.E = p_to_KE(p_CM, m_triton);
    n6Li_a.E = p_to_KE(p_CM, m_alpha);
}

//------------------------------------

REGISTER_EXECLET(testKinematics) {

    printf("\n");
    testRelKin();

    Li6Gen LG;
    printf("\n\n* n + 6Li ->\n");
    LG.n6Li_a.display();
    LG.n6Li_t.display();

    printf("\n\nEnergy/momentum conversions, numerically stable in nonrel. limit:\n");
    for(double m: {0., 1., 10., 100., 1e4, 1e6, 1e7, 1e8, 1e9, 1e99}) {
        double x = sqrt(1. + m*m) - m;
        printf("p = 1, m = %g\tKE = %g\t(naive: %g)\n", m, p_to_KE(1., m), x);
    }

    printf("\n\nbeta/gamma conversions, numerically stable in nonrel. limit:\n");
    for(double b: {0., 1e-2, 1e-5, 1e-6, 1e-7, 1e-8, 1e-9, 1e-99}) {
        auto gm1 = beta_to_gammaM1(b);
        printf("beta = %g\tgamma = 1 + %g\t(naive: 1 + %g)\troundtrip error %g\n",
               b, gm1, beta_to_gamma(b)-1, (gammaM1_to_beta(gm1)-b)/(b? b : 1));
    }

    printf("\n\nBoost composition round-trips:\n");
    auto L0 = Lorentz_boost::from_beta(0.8);
    L0.display();
    (L0 * L0.inverse()).display();
    (L0.inverse() * L0).display();
    (L0 / L0).display();

}
