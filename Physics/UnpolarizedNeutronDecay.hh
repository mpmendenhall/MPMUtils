/// \file UnpolarizedNeutronDecay.hh Neutron beta decay event generator with three-body kinematics plus radiative decay
// This file was produced under the employ of the United States Government,
// and is consequently in the PUBLIC DOMAIN, free from all provisions of
// US Copyright Law (per USC Title 17, Section 105).
//
// -- Michael P. Mendenhall, 2015

#ifndef UNPOLARIZEDNEUTRONDECAY_HH
#define UNPOLARIZEDNEUTRONDECAY_HH

#include "UnpolarizedBeta.hh"
#include <cstddef>
#include <cassert>
#include <stdint.h>

/// virtual class for providing random number vectors for decay kinematics
class NKine_Rndm_Src {
public:
    /// constructor
    NKine_Rndm_Src(): u(u0+3) { }
    /// destructor
    virtual ~NKine_Rndm_Src() { }
    
    /// get next random u[8] for electron, nu, gamma kinematics
    virtual void next() = 0;
    /// number of random slots supplied in u
    virtual size_t n_random() const = 0;
        
    double u0[11];      ///< kinematics random array, with optional first 3 position
    double* u;          ///< kinematics array starting at u0[3]
};

/// Base class for neutron decay kinematics generators
class NeutronDecayKinematics {
public:
    /// Constructor
    NeutronDecayKinematics(NKine_Rndm_Src* R): myR(R) { assert(R); }
    
    const double G_F = 1.1663787e-17;   ///< Fermi coupling constant, [/keV^2]
    const double G2_V = G_F*G_F*0.94920; ///< |G_V|^2 = |V_ud G_F g_V|^2
    const double m = m_n;               ///< initial nucleus mass
    const double m_2 = m_e;             ///< mass of charged lepton
    const double Delta = delta_mn_mp;   ///< decay energy m - m_f;
    
    /// Generate weighted event
    virtual void gen_evt_weighted() = 0;
    /// calculate cos theta between electron, neutrino
    double cos_theta_e_nu() const { return dot3(n_1, n_2); }
    /// calcualte "proton inferred" cos theta between electron, neutrino
    double proton_ctheta() const;
    /// radiative correction weight from fit to [Glu93] tables 
    double Gluck93_radcxn_wt() const;
    /// recoil and weak magnetism correction weight
    double B59_rwm_cxn_wt() const;
    
    double E_2;         ///< electron total energy [keV]
    double p_2;         ///< electron momentum magnitude [keV/c]
    double n_2[3];      ///< electron unit direction
    double beta;        ///< electron velocity v/c
    
    double E0_1;        ///< antineutrino energy in center-of-mass frame [keV]
    double E_1;         ///< antineutrino energy minus photon [keV]
    double p_1;         ///< neutrino momentum magnitude [keV/c]
    double n_1[3];      ///< neutrino momentum unit direction
    
    double K = 0;       ///< hard photon energy
    double n_gamma[3];  ///< gamma unit direction
    
    double p_f[3];      ///< recoil nucleon momentum
    double mag_p_f;     ///< magnitude of recoil momentum
    
    double evt_w;       ///< calculated event weight for kinematics
    
    double pt2_max = 0; ///< optional limit on maximum electron transverse momentum
    double c_2_min;     ///< optional minimum electron cos theta, calculated from pt2_max
    double c_2_wt;      ///< extra weight for c_2 selection
    
    /// number of random entries required
    virtual size_t n_random() const = 0;  
    
protected:
    
    NKine_Rndm_Src* myR;        ///< random number source
    
    /// calculate proton kinematics from electron, neutrino, gamma
    void calc_proton();
    /// unit vector from z cosine and x,y azimuth
    static void n_from_angles(double c, double phi, double n[3]);
    /// 3-vector dot product
    static inline double dot3(const double a[3], const double b[3]) { return a[0]*b[0] + a[1]*b[1] + a[2]*b[2]; }
};

/// Angular-uncorrelated (a=0), uncorrected 3-body neutron decay generator (weight = 1 efficient)
class N3BodyUncorrelated: public NeutronDecayKinematics {
public:
    /// Constructor
    N3BodyUncorrelated(NKine_Rndm_Src* R);
    /// Generate weighted event
    virtual void gen_evt_weighted() override;

    double c_1, c_2;            ///< phase-space cosines
    double phi_1, phi_2;        ///< phase-space azimuths
    
    /// number of random entries required
    virtual size_t n_random() const override { return 5; }
    
protected:
    static const size_t NPTS = 16384;   ///< lookup table dimensions
    double invcdf[NPTS+4];              ///< inverse CDF lookup table with interpolation guard entries
};

/// Math for re-assigning "natural" weighted splitting p, (1-p) to re-weighted q, (1-q)
class Reweighter {
public:
    Reweighter(double pp = 0, double qq = 0): p(pp), q(qq) { }
    double p;
    double q;
    double p_wt() const { return p/q; }
    double np_wt() const { return (1-p)/(1-q); }
};

/// Event generator for unpolarized neutron decays, including radiative corrections
/// Implementation of F. Gl\"uck, Computer Physics Communications 101 (1997) 223--231 ,
/// plus coulomb, recoil, and weak magnetism corrections
class Gluck_beta_MC: public NeutronDecayKinematics {
public:
    /// constructor
    Gluck_beta_MC(NKine_Rndm_Src* R, double M2_F = 1, double M2_GT = 3): NeutronDecayKinematics(R),
    zeta(M2_F + lambda*lambda*M2_GT), a( (M2_F - lambda*lambda*M2_GT/3.)/zeta ) { calc_rho(); }
        
    /// Generate weighted event
    virtual void gen_evt_weighted() override;
    
    /// Show "efficiency" of MC (5.22)
    void showEffic();
    /// test calculate hard photon decay probability by MC
    void test_calc_P_H(size_t n_sim = 1000000);
    
    /// additional recoil, weak magnetism weight factor, from Bilenki'i 1959 Eq. (10)
    double rwm_cxn() const;
    /// additional Coulomb correction weight factor
    double coulomb_cxn() const { return WilkinsonF0(1, E_2/m_e); }
    /// test electron spectrum shape factor against Sirlin (alpha/2pi)*g (4.15)
    double recalc_Sirlin_g_a2pi(double E_e);
    
    const double C_S = 0.001;           ///< hard photon production cutoff fraction 
    const double zeta;                  ///< spectrum weighting (2.13)
    const double a;                     ///< a_0 (2.13)
        
    double rho_H;       ///< hard decay rate (5.16)
    double rho_0;       ///< 0^th order decay rate (5.18)
    double rho_VS;      ///< soft photon decay rate (5.20)
    double rho_0VS;     ///< soft decay rate = rho_0 + rho_VS;
    double r_rho;       ///< total radiative correction to decay rate (6.1)
    double r_H;         ///< hard photon contribution to total radiative correction
    Reweighter P_H;     ///< probability of hard photon decay (5.21)
    double E_0VS;       ///< MC efficiency for virtual-soft events (5.22)
    double E_H;         ///< MC efficiency for hard decay events (5.22)
    
    double c_gamma;     ///< cos(theta_gamma) (5.6)
    double c_1, c_2;    ///< phase-space cosines
    double phi_gamma;   ///< gamma azimuth relative to electron
    double phi_1, phi_2;///< phase-space azimuths
    
    double M_0;         ///< uncorrected decay amplitude (2.11, 2.12)
    double Mtilde;      ///< (3.2)
    double M_VS;        ///< virtual and soft brem amplitude (3.9)
    double M_BR;        ///< hard brem amplitude (4.4)
    
    double evt_w0;      ///< event weight without radiative corrections
    
    /// number of random entries required
    virtual size_t n_random() const override { return 8; }
    
protected:

    double N;           ///< N(beta) (3.3)
    double omega;       ///< hard photon cutoff energy (3.8)
    double np_2[3];     ///< coordinate transform vector (5.10)
    double npp_2[3];    ///< coordinate transform vector (5.10)
    double V_g;         ///< re-weighting function integral (5.15)
    
    double w_avg = 1.11164e-30;         ///< average value of w, (5.23) and (5.16)
    double Wavg_0VS = 7.83328e-10;      ///< average value of W_0VS (5.23)
    double w_max = 0;           ///< maximum observed value of w
    double Wmax_0VS = 0;        ///< maximum observed value of W_0VS
    int64_t n_H = 0;            ///< number of hard brem events generated
    int64_t n_S = 0;            ///< number of soft brem events generated
    double sum_w = 0;           ///< sum of hard brem weights
    double sum_W_0VS = 0;       ///< sum of soft brem weights
    
    /// choose initial un-weighted kinematics
    void propose_kinematics();
    /// calculate electron vector and relative coordinates
    void calc_n_2();
    /// calculate unit vector specified relative to n_2
    void vec_rel_n_2(double c, double phi, double* v) const;
    /// calculate beta and N(beta)
    void calc_beta_N();
    
    /// corrected spectrum probability for virtual and soft brem photons
    double calc_soft();
    /// corrected spectrum probability for hard photon production
    double calc_hard_brem();
    
    /// virtual and soft brem correction (3.10)
    double z_VS() const;
    /// hard brem correction (4.14)
    double z_H() const;
    /// calculate rho_0VS, rho_H non-gamma-emitting rate
    void calc_rho();
};

/// Implementation of F. Gluck, Phys Rev D 47(7), pp.2840--2848, 1993
class Gluck93_Distribution {
public:
    /// Constructor
    Gluck93_Distribution() { }
    
    const double G_F = 1.1663787e-17;   ///< Fermi coupling constant, [/keV^2]
    const double G2_V = G_F*G_F*0.94920; ///< |G_V|^2 = |V_ud G_F g_V|^2
    const double m_i = m_n;             ///< initial nucleus mass
    const double m_f = m_p;             ///< final nucleus mass
    const double m_2 = m_e;             ///< mass of charged lepton
    const double Delta = delta_mn_mp;   ///< decay energy m_i - m_f;
    const double E_2m = Delta - (Delta*Delta-m_2*m_2)/(2*m_i);  ///< (2.3) recoil-corrected endpoint
    const double kappa = (1.792847356-(-1.91304273))/2;         ///< difference in _anomalous_ magnetic moments
    const double a0 = calc_a0();        ///< a_0 base asymmetry
    
    /// Calculate electron/(inferred)nu distribution with full corrections
    double calc_Wenu_0Ca(double E_2, double c);
    /// Calculate electron/proton phase space, including recoil-order but not radiative corrections
    double calc_W_0C(double E_2, double E_f, double c_f = 0);
    /// Calculate electron/nu phase space uncorrected
    double calc_Wenu_0(double E_2, double c);
    
    double Wenu_0Ca;    ///< (4.2) electron/(inferred)nu phase space with all corrections
    double Wenu_0;      ///< (4.6) zeroth-order approximated elecrton/nu distribution
    double W_0C;        ///< (3.1) electron/proton phase space with Fermi function
    double dEfc_dc;     ///< (4.4)
    double p_2;         ///< | electron momentum |
    double beta;        ///< electron velocity/c
};


/// Recoil, weak magnetism weight factor (1 + delta rwm)
/// from Bilenkii et. al., JETP 37 (10), No. 6, 1960
/// formula in equation (10), with 1+3*lambda^2 factored out
/// and also dividing out (1+beta*a0*cth) to avoid double-counting 'a' contribution.
/// lambda = |lambda| > 0 sign convention.
double B59_rwm_cxn(double E, double cos_thn);

/// Radiative correction according to Garcia, Maya, Phys. Rev. D, 1978 (irrelevant for lab (proton) observables)
double GM78_radiative_cxn(double E, double cos_thn);

/// Lab-observable radiative correction r_{e\nu}
/// Parametrized fit to table in Gluck, Phys. Rev. D 47 (1993), 2840-2848
/// x in [0,1] is rescaled electron kinetic energy; c is "proton inferred" \cos \theta_{e\nu}
inline double Gluck93_r_enu(double x, double c) { return 0.002 + 0.014*x + (-0.009+0.179*x)*c; }


#endif
