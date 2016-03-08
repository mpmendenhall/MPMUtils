/// \file BetaSpectrumGenerator.cc

#include "BetaSpectrumGenerator.hh"
#include "UnpolarizedBeta.hh"

BetaSpectrumGenerator::BetaSpectrumGenerator(double a, double z, double ep): A(a), Z(z), EP(ep),
W0((EP+m_e)/m_e), R(pow(A,1./3.)*neutron_R0), M0(fabs(Z)*proton_M0+(A-fabs(Z))*neutron_M0),
forbidden(0), M2_F(0), M2_GT(1) { }

double BetaSpectrumGenerator::spectrumCorrectionFactor(double W) const {
    double c = WilkinsonF0(Z,W,R);              // Fermi function Coulomb
    c *= WilkinsonL0(Z,W,R);                    // Nonzero charge radius effect on Coulomb correction
    c *= CombinedC(Z,W,M2_F,M2_GT,W0,R);        // electron/nucleon nonzero size wavefunction convolution
    c *= WilkinsonQ(Z,W,W0,M0);                 // Recoil effect on Coulomb correction (tiny tiny!)
    c *= (1.+Wilkinson_g_a2pi(W,W0,M0));        // outer radiative corrections
    if(A==1 && Z==1) {
        c *= (1.+Bilenkii59_RWM(W));            // recoil + WM for free neutron
    } else {
        c *= CombinedR(W,M2_F,M2_GT,W0,M0);     // recoil effect on phase space
    }
    // first forbidden Axial-Vector decays
    if(forbidden==1 && M2_GT>0 && M2_F==0)
        c *= Davidson_C1T(W, W0, Z, R);
    // Cs137 second-forbidden decay
    if(forbidden==2 && A==137)
        c *= Behrens_Cs137_C(W, W0);
    
    return c;
}

double BetaSpectrumGenerator::decayProb(double KE) const {
    double W = (KE+m_e)/m_e;
    return plainPhaseSpace(W,W0)*spectrumCorrectionFactor(W);
}
