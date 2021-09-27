/// \file NuclEvtGen.hh Nuclear decay scheme event generator
// -- Michael P. Mendenhall

#ifndef NUCLEVTGEN_HH
#define NUCLEVTGEN_HH

#include "ElectronBindingEnergy.hh"
#include "BetaSpectrumGenerator.hh"
#include "TF1_Quantiles.hh"
#include "FloatErr.hh"

#include <set>
using std::set;
#include <limits>
#include <stdio.h>

/// random event selector
class PSelector {
public:
    /// constructor
    PSelector() { cumprob.push_back(0); }
    /// add a probability
    void addProb(double p) { cumprob.push_back(p+cumprob.back()); }
    /// select partition for given input (random if not specified); re-scale input to partition range to pass along to sub-selections
    unsigned int select(double* x = nullptr) const;
    /// get cumulative probability
    double getCumProb() const { return cumprob.back(); }
    /// get number of items
    unsigned int getN() const { return cumprob.size()-1; }
    /// get probability of numbered item
    double getProb(unsigned int n) const;
    /// scale all probabilities
    void scale(double s) { for(auto& p: cumprob) p *= s; }

protected:
    vector<double> cumprob; ///< cumulative probabilites
};

/// generate an isotropic random direction, from optional random in [0,1]^2
void randomDirection(double& x, double& y, double& z, double* rnd = nullptr);

/// Nuclear energy level
class NucLevel {
public:
    /// constructor
    explicit NucLevel(const Stringmap& m);
    /// print info
    void display(bool verbose = false) const;
    /// scale probabilities
    void scale(double s) { fluxIn *= s; fluxOut *= s; }

    /// ordering operator
    bool operator<(const NucLevel& b) const { return E < b.E; }

    string name;        ///< name for this level
    unsigned int A;     ///< nucleus A
    unsigned int Z;     ///< nucleus Z
    unsigned int n;     ///< level number
    double E;           ///< energy [MeV]
    double hl;          ///< half-life [s]
    string jpi;         ///< spin/parity
    double fluxIn;      ///< net flux into level
    double fluxOut;     ///< net flux out of level
};

/// primary particle types
/// using PDG numbering scheme, http://pdg.lbl.gov/2012/reviews/rpp2012-rev-monte-carlo-numbering.pdf
enum DecayType_t {
    D_GAMMA = 22,
    D_ELECTRON = 11,
    D_POSITRON = -11,
    D_NUEBAR = -12,
    D_ALPHA = 1000020040,
    D_NONEVENT = 0
};

/// string name of particle types
string particleName(DecayType_t t);
/// decay type from particle name
DecayType_t particleType(const string& s);


/// specification for a decay particle
struct NucDecayEvent {
    /// randomize momentum direction
    void randp(double* rnd = nullptr) { randomDirection(p[0],p[1],p[2],rnd); }

    unsigned int eid = 0;       ///< event ID number
    double E = 0;               ///< particle energy [MeV]
    double p[3];                ///< particle momentum direction
    double x[3];                ///< vertex position [arb.]
    DecayType_t d = D_NONEVENT; ///< particle type [PDG/Geant PID]
    double t = 0;               ///< time of event [s]
    double w = 1;               ///< weighting for event
};

/// Atom/electron information
class DecayAtom {
public:
    /// constructor
    explicit DecayAtom(BindingEnergyTable const* B);
    /// load Auger data from Stringmap
    void load(const Stringmap& m);
    /// generate Auger K probabilistically
    void genAuger(vector<NucDecayEvent>& v);
    /// display info
    void display(bool verbose = false) const;

    BindingEnergyTable const* BET;      ///< binding energy table
    double Eauger;                      ///< Auger K energy [MeV]
    double Iauger;                      ///< intensity of Auger electron emissions
    double Ikxr;                        ///< intensity of k X-Ray emissions
    double ICEK;                        ///< intensity of CE K events
    double IMissing;                    ///< intensity of un-accounted for Augers (from initial capture)
    double pAuger;                      ///< probability of auger given any opening
};

/// Transition base class
class TransitionBase {
public:
    /// constructor
    TransitionBase(NucLevel& f, NucLevel& t): from(f), to(t), Itotal(0) {}
    /// destructor
    virtual ~TransitionBase() {}
    /// display transition line info
    virtual void display(bool verbose = false) const;

    /// select transition outcome
    virtual void run(vector<NucDecayEvent>&, double* = nullptr) { }

    /// return number of continuous degrees of freedom needed to specify transition
    virtual unsigned int getNDF() const { return 2; }

    /// scale probability
    virtual void scale(double s) { Itotal *= s; }

    /// get probability of removing an electron from a given shell
    virtual double getPVacant(unsigned int) const { return 0; }
    /// how many of given electron type were knocked out
    virtual unsigned int nVacant(unsigned int) const { return 0; }

    DecayAtom* toAtom;  ///< final state atom info
    NucLevel& from;     ///< level this transition is from
    NucLevel& to;       ///< level this transition is to
    double Itotal;      ///< total transition intensity
};


/// Gamma transition with possible conversion electrons
class ConversionGamma: public TransitionBase {
public:
    /// constructor
    ConversionGamma(NucLevel& f, NucLevel& t, const Stringmap& m);
    /// select transition outcome
    void run(vector<NucDecayEvent>& v, double* rnd = nullptr) override;
    /// display transition line info
    void display(bool verbose = false) const override;
    /// get total conversion efficiency
    double getConversionEffic() const;
    /// get probability of knocking conversion electron from a given shell
    double getPVacant(unsigned int n) const override { return n<shells.getN()-1?shells.getProb(n):0; }
    /// get whether said electron was knocked out
    unsigned int nVacant(unsigned int n) const override { return int(n)==shell; }
    /// shell weighted average energy
    double shellAverageE(unsigned int n) const;
    /// line weighted average
    float_err averageE() const;
    /// scale probability
    void scale(double s) override;

    double Egamma;      ///< gamma energy [MeV]
    int shell;          ///< selected conversion electron shell
    int subshell;       ///< selected conversion electron subshell
    double Igamma;      ///< total gamma intensity

protected:
    PSelector shells;           ///< conversion electron shells
    vector<float> shellUncert;  ///< uncertainty on shell selection probability
    vector<PSelector> subshells;///< subshell choices for each shell
};

/// electron capture transitions
class ECapture: public TransitionBase {
public:
    /// constructor
    ECapture(NucLevel& f, NucLevel& t): TransitionBase(f,t) {
        if(t.A != f.A || t.Z+1 != f.Z || t.E >= f.E)
            throw std::runtime_error("Invalid ECapture transition");
    }
    /// select transition outcome
    void run(vector<NucDecayEvent>&, double* rnd = nullptr) override;
    /// display transition line info
    void display(bool verbose = false) const override { printf("Ecapture "); TransitionBase::display(verbose); }
    /// get probability of removing an electron from a given shell
    double getPVacant(unsigned int n) const override { return n==0? toAtom->IMissing : 0; }
    /// get whether said electron was knocked out
    unsigned int nVacant(unsigned int n) const override { return n==0? isKCapt : 0; }

    /// return number of continuous degrees of freedom needed to specify transition
    unsigned int getNDF() const override { return 0; }

    bool isKCapt;       ///< whether transition was a K capture
};

/// alpha decay transitions
class AlphaDecayTrans: public TransitionBase {
public:
    /// constructor
    AlphaDecayTrans(NucLevel& f, NucLevel& t, const Stringmap& m);
    /// select transition outcome
    void run(vector<NucDecayEvent>&, double* rnd = nullptr) override;
    /// display transition line info
    void display(bool verbose = false) const override;
    /// return number of continuous degrees of freedom needed to specify transition
    unsigned int getNDF() const override { return 2; }

    double Ealpha; ///< alpha kinetic energy [MeV]
};

/// beta decay transitions
class BetaDecayTrans: public TransitionBase {
public:
    /// constructor
    BetaDecayTrans(NucLevel& f, NucLevel& t, unsigned int forbidden = 0);
    /// destructor
    ~BetaDecayTrans() {  delete betaQuantiles; }

    /// select transition outcome
    void run(vector<NucDecayEvent>& v, double* rnd = nullptr) override;
    /// display transition line info
    void display(bool verbose = false) const override;

    /// return number of continuous degrees of freedom needed to specify transition
    unsigned int getNDF() const override { return 3; }

    bool positron;                      ///< whether this is positron decay
    BetaSpectrumGenerator BSG;          ///< spectrum shape generator

protected:
    /// evaluate beta spectrum probability as function of KE [MeV]
    double evalBeta(double* x, double*) { return BSG.decayProb(*x); }

    TF1 betaTF1;                        ///< TF1 for beta spectrum shape
    TF1_Quantiles* betaQuantiles;       ///< inverse CDF of beta spectrum shape for random point selection
};

/// Decay system
class NucDecaySystem {
public:
    /// constructor from specification file
    NucDecaySystem(const SMFile& Q, const BindingEnergyLibrary& B, double t = std::numeric_limits<double>::infinity());
    /// destructor
    ~NucDecaySystem();
    /// set cutoff lifetime for intermediate states
    void setCutoff(double t);
    /// effective number of starting points after time cuts
    double nStarts() const { return lStart.getCumProb(); }
    /// display transitions summary
    void display(bool verbose = false) const;
    /// display list of levels
    void displayLevels(bool verbose = false) const;
    /// display list of transitions
    void displayTransitions(bool verbose = false) const;
    /// display list of atoms
    void displayAtoms(bool verbose = false) const;
    /// generate a chain of decay events starting from level n, starting time offset t0
    void genDecayChain(vector<NucDecayEvent>& v, double* rnd = nullptr,
                       unsigned int n = std::numeric_limits<unsigned int>::max(), double t0 = 0);
    /// rescale all probabilities
    void scale(double s);

    /// return number of degrees of freedom needed to specify decay from given level
    unsigned int getNDF(unsigned int n = std::numeric_limits<unsigned int>::max()) const;

    /// LaTeX name for generator
    string fancyname;

protected:
    /// get index for named level
    unsigned int levIndex(const string& s) const;
    /// get atom info for given Z
    DecayAtom* getAtom(unsigned int Z);
    /// add a transition
    void addTransition(TransitionBase* T);
    /// check against circular references
    void circle_check(unsigned int n, set<unsigned int>& passed, set<unsigned int> pnts = set<unsigned int>()) const;
    /// sum fluxIn, fluxOut for specified level
    void sumFluxInOut(size_t l);
    /// normalize input/output fluxes to consistency
    void normalizeFluxInOut();

    BindingEnergyLibrary const&  BEL;           ///< electron binding energy info
    double tcut;                                ///< cutoff time for splitting events
    vector<NucLevel> levels;                    ///< levels, enumerated
    map<string,unsigned int> levelIndex;        ///< energy levels by name
    PSelector lStart;                           ///< selector for starting level (for breaking up long decays)
    vector<PSelector> levelDecays;              ///< probabilities for transitions from each level
    map<unsigned int, DecayAtom*> atoms;        ///< atom information
    vector<TransitionBase*> transitions;        ///< transitions, enumerated
    vector< vector<TransitionBase*> > transIn;  ///< transitions into each level
    vector< vector<TransitionBase*> > transOut; ///< transitions out of each level
};

/// manager for loading decay event generators
class NucDecayLibrary {
public:
    /// constructor
    explicit NucDecayLibrary(const string& datp, double t = std::numeric_limits<double>::infinity());
    /// destructor
    ~NucDecayLibrary();
    /// check if generator is available
    bool hasGenerator(const string& gennm);
    /// get decay generator by name
    NucDecaySystem& getGenerator(const string& gennm);

    string datpath;                     ///< path to data folder
    double tcut;                        ///< event generator default cutoff time
    BindingEnergyLibrary  BEL;          ///< electron binding energy info

protected:
    map<string,NucDecaySystem*> NDs;    ///< loaded decay systems
    set<string> cantdothis;             ///< list of decay systems that can't be loaded
};

/// class for throwing from large list of gammas
class GammaForest {
public:
    /// Constructor, with conversion factor to MeV
    explicit GammaForest(const string& fname, double E2MeV = 1);
    /// get total cross section
    double getCrossSection() const { return gammaProb.getCumProb(); }
    /// generate cluster of gamma decays
    void genDecays(vector<NucDecayEvent>& v, double n = 1.0);
protected:
    vector<double> gammaE;      ///< gamma energies [MeV]
    PSelector gammaProb;        ///< gamma probabilities selector
};

#endif
