/// \file BetaSpectrumGenerator.hh Class for calculating beta spectrum shape
// Michael P. Mendenhall

#ifndef BETASPECTRUMGENERATOR_HH
#define BETASPECTRUMGENERATOR_HH

/// Unpolarized beta decay spectrum calculating class
class BetaSpectrumGenerator {
public:
    /// Constructor, with endpoint in [MeV]
    BetaSpectrumGenerator(double a, double z, double ep);

    /// shape correction to basic phase space
    double spectrumCorrectionFactor(double W) const;

    /// decay probability at given KE [MeV]
    double decayProb(double KE) const;

    /// display settings
    void display() const;
    /// display correction factors at energy [MeV]
    void showCorrections(double KE) const;

    double A;                   ///< number of nucleons
    double Z;                   ///< number of protons
    double EP;                  ///< endpoint kinetic energy, MeV
    double W0;                  ///< endpoint total energy, m_e*c^2
    double R;                   ///< effective nuclear radius
    double M0;                  ///< nuclear mass, m_e*c^2
    unsigned int forbidden;     ///< "forbidden" level of decay
    double M2_F;                ///< |M_F|^2 Fermi decay matrix element
    double M2_GT;               ///< |M_GT|^2 Gamov-Teller decay matrix element
};

#endif
