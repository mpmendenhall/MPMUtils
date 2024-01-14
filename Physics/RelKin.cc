/// @file RelKin.cc

#include "RelKin.hh"
#include <stdio.h>

double p_2body(double m1, double m2, double KE) {
    double x = KE * (KE + 2*(m1 + m2));
    return sqrt(x*(x + 4*m1*m2)) / (2*(m1 + m2 + KE));
}

void Lorentz_boost::display() const {
    printf("Lorentz boost with gamma = 1 + %g, beta = %g", gammaM1, beta);
    double dgamma = gammaM1 - beta_to_gammaM1(beta);
    if(dgamma) printf(" ** Accumulated inconsistency gamma - gamma(beta) = %g", dgamma);
    printf("\n");
}

void Lorentz_boost::boost(double& v0, double& v3) const {
    double vv0 = v0;
    double g = gamma();
    v0 =  g*vv0 - beta*g*v3;
    v3 = -beta*g*vv0 + g*v3;
}

void Lorentz_boost::unboost(double& v0, double& v3) const {
    double vv0 = v0;
    double g = gamma();
    v0 =  g*vv0 + beta*g*v3;
    v3 =  beta*g*vv0 + g*v3;
}

void Lorentz_boost::operator*=(const Lorentz_boost& b) {
    double gm1 = (gammaM1*b.gammaM1 + gammaM1 + b.gammaM1)*(1 + beta * b.beta) + beta * b.beta;
    beta = (beta + b.beta) * b.gamma() * gamma() / (gm1 + 1);
    gammaM1 = gm1;
}

void Lorentz_boost::operator/=(const Lorentz_boost& b) {
    double gm1 = (gammaM1*b.gammaM1 + gammaM1 + b.gammaM1)*(1 - beta * b.beta) - beta * b.beta;
    beta = (beta - b.beta) * b.gamma() * gamma() / (gm1 + 1);
    gammaM1 = gm1;
}

Lorentz_boost Lorentz_boost::to_projectile_CM(double& KE, double mProj, double mTarg, bool forward) {
    double M = mProj + mTarg;       // total rest mass
    double pL = KE_to_p(KE, mProj); // lab-frame projectile momentum
    if(!forward) pL *= -1;
    auto L = from_beta(pL/(KE + M));// boost to CM velocity in lab frame
    double g = L.gammaM1 + 1;
    KE = g*KE + L.gammaM1*M - L.beta*g*pL;
    return L;
}

// check solution branch consistency
double check_pz_branch(double E, double m, double dz, double pz, const Lorentz_boost& LB) {
    double Ep = LB.gamma()*(E - LB.beta*pz);
    double pp = sqrt(Ep*Ep -m*m);
    return LB.gamma()*(pz - LB.beta*E) - dz*pp;
}

double Lorentz_boost::pz_CM_from_lab_direction(double E_CM, double m, double dz_lab) const {
    // kinematics equations -> a*p_z^2 + b*p_z + c = 0
    double gm2 = gamma()*gamma();
    double b2 = beta*beta;
    double dz2 = dz_lab * dz_lab;
    double a = -gm2*(1-dz2*b2);
    double b = 2 * beta * gm2 * (1-dz2) * E_CM;
    double c = gm2*(dz2-b2)*E_CM*E_CM - dz2*m*m;
    double u = sqrt(b*b - 4*a*c);

    // calculate two solution branches pz1,pz2 for quadratic;
    // choose the one consistent with kinematics equations
    double pz1 = (u - b)/(2*a);
    double pz2 = (-u - b)/(2*a);
    double pb1 = check_pz_branch(E_CM, m, dz_lab, pz1, *this);
    double pb2 = check_pz_branch(E_CM, m, dz_lab, pz2, *this);

    return fabs(pb1) < fabs(pb2)? pz1 : pz2;
}

double Lorentz_boost::dz_CM_from_lab_direction(double p_CM, double m, double dz_lab) const {
    return pz_CM_from_lab_direction(sqrt(p_CM * p_CM + m * m), m,  dz_lab)/p_CM;
}


void testRelKin() {
    double TKE = 100;
    double m0 = 50;
    double m1 = 100;

    printf("2-body decay into %g MeV/c^2, %g MeV/c^2 with %g MeV total KE\n", m0, m1, TKE);
    double p = p_2body(m0, m1, TKE);
    printf("Each has momentum %g MeV/c, ", p);
    double E0 = p_to_KE(p, m0);
    double E1 = p_to_KE(p, m1);
    printf("and kinetic energies %g + %g = %g MeV\n", E0, E1, E0+E1);

    double KEcm = TKE;
    auto LB = Lorentz_boost::to_projectile_CM(KEcm, m0, m1);
    double p0Lab = KE_to_p(TKE, m0);
    TKE = p_to_KE(p0Lab, m0);
    printf("\nFor a %g MeV/c^2 projectile at at %g MeV KE (p = %g MeV/c) incident on a %g MeV/c^2 target,\n", m0, TKE, p0Lab, m1);
    printf("The CM frame is boosted by beta = %g, gamma = 1 + %g, with %g MeV total kinetic energy.\n", LB.beta, LB.gammaM1, KEcm);

    double p0 = LB.boost_p(m0, p0Lab, p0Lab*p0Lab);
    double p1 = LB.boost_p(m1, 0, 0);
    printf("In the CM frame, momenta are %g and %g, ", p0, p1);
    double E0cm = p_to_KE(p0, m0);
    double E1cm = p_to_KE(p1, m1);
    printf("with energies %g + %g = %g.\n", E0cm, E1cm, E0cm + E1cm);
}

