/// \file PolarizedBetaAsym.cc
#include "PolarizedBetaAsym.hh"
#include <stdio.h>
#include <cmath>
#include <vector>
#include <map>

#ifdef USE_ROOT_MATH
#include <TMath.h>
#define Gamma(x) TMath::Gamma(x)
#else
#define Gamma(x) tgamma(x)
#endif

double shann_h_a2pi(double KE, double KE0, double m) {
    if(KE<=0 || KE>=KE0)
        return 0;
    double b = beta(KE,m);
    double E = KE+m;
    double E0 = KE0+m;
    double athb = atanh(b);
    return (3.*log(m_p/m)-3./4.
    + 4.*(athb/b-1.)*((E0-E)/(3.*E*b*b)+(E0-E)*(E0-E)/(24*E*E*b*b)-3./2.+log(2.*(E0-E)/m))
    + 4./b*SpenceL(2.*b/(1.+b))+4*athb/b*(1-athb)
    )*alpha/(2.*M_PI);
}

double shann_h_minus_g_a2pi(double W, double W0) {
    if(W>=W0 || W<=1)
        return 0;
    double b = sqrt(W*W-1)/W;
    double athb = atanh(b);
    return ( 4.*(athb/b-1.)*(1/(b*b)-1)*(W0-W)/(3*W)*(1+(W0-W)/(8*W))
    +athb/b*(2.-2.*b*b) - (W0-W)*(W0-W)/(6.*W*W) )*alpha/(2.*M_PI);
}

double WilkinsonACorrection(double W) {
    const double W0 = neutronBetaEp/m_e+1.;
    const double A_uM = (lambda+delta_mu)/(lambda*(1.-lambda)*(1.+3.*lambda*lambda)*m_p/m_e);
    const double A_1 = lambda*lambda+2.*lambda/3.-1./3.;
    const double A_2 = -lambda*lambda*lambda-3.*lambda*lambda-5.*lambda/3.+1./3.;
    const double A_3 = 2.*lambda*lambda*(1.-lambda);
    return A_uM*(A_1*W0+A_2*W+A_3/W);
}
