/// \file BetaSpectrumGenerator.cc

#include "BetaSpectrumGenerator.hh"
#include "UnpolarizedBeta.hh"
#include <stdio.h>

BetaSpectrumGenerator::BetaSpectrumGenerator(double a, double z, double ep): A(a), Z(z), EP(ep),
W0((EP+m_e)/m_e), R(pow(A,1./3.)*neutron_R0), M0(fabs(Z)*proton_M0+(A-fabs(Z))*neutron_M0),
forbidden(0), M2_F(0), M2_GT(1) { }

double BetaSpectrumGenerator::spectrumCorrectionFactor(double W) const {
    double c = WilkinsonF0(Z,W,R);              // Fermi function Coulomb
    if(Z > 0) c *= WilkinsonL0(Z,W,R);          // Nonzero charge radius effect on Coulomb correction
    c *= CombinedC(Z,W,M2_F,M2_GT,W0,R);        // electron/nucleon nonzero size wavefunction convolution
    c *= WilkinsonQ(Z,W,W0,M0);                 // Recoil effect on Coulomb correction (tiny tiny!)
    c *= (1.+Wilkinson_g_a2pi(W,W0,M0));        // outer radiative corrections
    if(A==1 && Z==1) c *= (1.+Bilenkii59_RWM(W));   // recoil + WM for free neutron
    else c *= CombinedR(W,M2_F,M2_GT,W0,M0);        // recoil effect on phase space

    // first forbidden Axial-Vector decays
    if(forbidden == 1 && M2_GT > 0 && M2_F == 0) c *= Davidson_C1T(W, W0, Z, R);
    // Cs137 second-forbidden decay branch
    if(forbidden == 2 && A == 137 && Z == 56) c *= Behrens_Cs137_C(W, W0);

    return c;
}

void BetaSpectrumGenerator::display() const {
    printf("Beta spectrum for A = %.2f, Z = %.2f, endpoint %.3f MeV\n", A, Z, EP);
}

void BetaSpectrumGenerator::showCorrections(double KE) const {
    double W = (KE + m_e)/m_e;
    printf("E = %g MeV\tW = %g\t(W0 = %g)\n", KE, W, W0);
    printf("S = %g\n", plainPhaseSpace(W,W0));
    printf("Fermi Function:       %g\n", WilkinsonF0(Z,W,R));
    if(Z > 0) printf("Charge radius:        %g\n", WilkinsonL0(Z,W,R) - 1);
    printf("Wavefunction overlap: %g\n", CombinedC(Z,W,M2_F,M2_GT,W0,R) - 1);
    printf("Recoil x Coulomb:     %g\n", WilkinsonQ(Z,W,W0,M0) - 1);
    printf("Outer radiative:      %g\n", Wilkinson_g_a2pi(W,W0,M0));

    if(A == 1 && Z == 1)
        printf("Recoil + WM:          %g\n", Bilenkii59_RWM(W));
    else
        printf("Recoil on PS:         %g\n", CombinedR(W,M2_F,M2_GT,W0,M0) - 1);

    if(forbidden == 1 && M2_GT > 0 && M2_F == 0)
        printf("1st Forbidden:        %g\n", Davidson_C1T(W, W0, Z, R) - 1);

    if(forbidden == 2 && A == 137 && Z == 56)
        printf("Cs137 shape:          %g\n", Behrens_Cs137_C(W, W0));
}

double BetaSpectrumGenerator::decayProb(double KE) const {
    double W = (KE + m_e)/m_e;
    if(KE <= 0 || W >= W0) return 0.;
    auto p = plainPhaseSpace(W,W0)*spectrumCorrectionFactor(W);
    if(!(p >= 0)) {
        printf("Warning: nonphysical beta probability %g\n", p);
        showCorrections(KE);
    }
    return p;
}
