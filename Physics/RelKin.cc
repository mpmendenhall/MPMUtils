/// \file RelKin.cc

#include "RelKin.hh"
#include <stdio.h>

double p22ke(double p2, double m) {
    double KE = sqrt(p2 + m*m) - m;
    return (p2 - KE*KE)/(2*m); // numerically stable for KE,p << m
}

double p_2body(double m1, double m2, double KE) {
    double x = KE*(KE+2*(m1+m2));
    return sqrt(x*(x+4*m1*m2))/(2*(m1+m2+KE));
}

double Lorentz_boost::boost(double v0, double& v3) const {
    //  g   -bg
    //    1
    //      1
    // -bg    g
    const double vv0 = v0;
    const double vv3 = v3;
    v0 =  gamma*vv0 - beta*gamma*vv3;
    v3 = -beta*gamma*vv0 + gamma*vv3;
    return v0;
}

double Lorentz_boost::unboost(double v0, double& v3) const {
    const double vv0 = v0;
    const double vv3 = v3;
    v0 =  gamma*vv0 + beta*gamma*vv3;
    v3 =  beta*gamma*vv0 + gamma*vv3;
    return v0;
}

double Lorentz_boost::projectileCM(double KE, double mProj, double mTarg) {
    double M = mProj + mTarg;       // total rest mass
    double pL = ke2p(KE, mProj);    // lab-frame projectile momentum
    beta = pL/(KE + M);             // cm velocity in lab frame
    gamma = b2gamma(beta);
    return gamma*KE + (gamma - 1)*M - beta*gamma*pL;
}


void testRelKin() {
    double TKE = 100;
    double m0 = 50;
    double m1 = 100;

    printf("2-body decay into %g MeV/c^2, %g MeV/c^2 with %g MeV total KE\n", m0, m1, TKE);
    double p = p_2body(m0, m1, TKE);
    printf("Each has momentum %g MeV/c, ", p);
    double E0 = p2ke(p, m0);
    double E1 = p2ke(p, m1);
    printf("and kinetic energies %g + %g = %g MeV\n", E0, E1, E0+E1);

    Lorentz_boost LB;
    double KEcm = LB.projectileCM(TKE, m0, m1);
    double p0Lab = ke2p(TKE, m0);
    TKE = p2ke(p0Lab, m0);
    printf("\nFor a %g MeV/c^2 projectile at at %g MeV KE (p = %g MeV/c) incident on a %g MeV/c^2 target,\n", m0, TKE, p0Lab, m1);
    printf("The CM frame is boosted by beta = %g, gamma = %g, with %g MeV total kinetic energy.\n", LB.beta, LB.gamma, KEcm);

    double p0 = LB.boost_p(m0, p0Lab, p0Lab*p0Lab);
    double p1 = LB.boost_p(m1, 0, 0);
    printf("In the CM frame, momenta are %g and %g, ", p0, p1);
    double E0cm = p2ke(p0, m0);
    double E1cm = p2ke(p1, m1);
    printf("with energies %g + %g = %g.\n", E0cm, E1cm, E0cm + E1cm);
}

