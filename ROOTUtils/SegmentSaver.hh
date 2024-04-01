/// @file SegmentSaver.hh Mechanism for loading and summing pre-defined histograms from file
// -- Michael P. Mendenhall, LLNL 2022

#ifndef SEGMENTSAVER_HH
#define SEGMENTSAVER_HH

#include "OutputManager.hh"
#include "SignalSink.hh"
#include "TCumulativeMap.hh"
#include "TermColor.hh"

#include <TFile.h>
#include <TVectorT.h>
#include <TH1.h>

#include <set>
using std::set;
#include <stdexcept>
#include <type_traits>

/// utility function to remove color axis data, to force re-draw with current dimensions
void resetZaxis(TH1* o);

/// class for saving, retrieving, and summing data from file
class SegmentSaver: public OutputManager, virtual public SignalSink {
public:
    /// constructor, optionally with input filename
    explicit SegmentSaver(OutputManager* pnt, const string& _path = "SegmentSaver", const string& inflName = "");
    /// destructor
    virtual ~SegmentSaver();

    /// change name, and subpaths if in parent
    virtual void rename(const string& nm);
    /// write items to current directory or subdirectory of provided
    TDirectory* writeItems(TDirectory* d = nullptr) override;

    /// get metadata string
    const string& getMeta(const string& k);
    /// set metadata string
    void setMeta(const string& k, const string& v) { xmeta[k] = v; }

    /// construct or retrieve saved TH1-derived class
    template<class T, typename... Args>
    void registerSaved(T*& o, const string& hname, Args&&... a) {
        if(saveHists.count(hname)) throw std::logic_error("Duplicate name '"+hname+"'"); // don't duplicate names!
        if(o) throw std::logic_error("Registration of '" + path + "/" + hname + "' would overwrite non-null pointer");
        o = tryLoad<T>(hname);
        if(!o) o = addObject(new T(hname.c_str(), std::forward<Args>(a)...));
        saveHists.emplace(hname, o);
        if(o->ClassName() == TString("TProfile") || o->ClassName() == TString("TProfile2D")) doNotScale.insert(o);
    }

    /// clone from template or restore from file a saved TH1-derived class
    template<class T, class U>
    typename std::enable_if<std::is_base_of<TH1, U>::value>::type registerSavedClone(T*& h, const string& hname, const U& hTemplate) {
        if(h) throw std::logic_error("Registration of '" + path + "/" + hname + "' would overwrite non-null pointer");
        h = static_cast<T*>(_registerSavedClone(hname, hTemplate));
    }

    /// construct or retrieve saved TCumulative-derived class
    template<class T, typename... Args>
    void registerTCumulative(T*& o, const string& hname, Args&&... a) {
        if(cumDat.count(hname)) throw std::logic_error("Duplicate name '"+hname+"'"); // don't duplicate names!
        if(o) throw std::logic_error("Registration of '" + path + "/" + hname + "' would overwrite non-null pointer");
        o = tryLoad<T>(hname);
        if(!o) o = addObject(new T(hname, std::forward<Args>(a)...));
        cumDat.emplace(hname, o);
    }

    /// construct or retrieve named cumulative from file (requires file-based constructor)
    template<class CUMDAT, typename... Args>
    void registerAccumulable(CUMDAT*& o, const string& onm, Args&&... a) {
        if(cumDat.count(onm)) throw std::runtime_error("Duplicate cumulative name '" + onm + "'");
        if(o) throw std::logic_error("Registration of '" + path + "/" + onm + "' would overwrite non-null pointer");
        if(dirIn) o = new CUMDAT(onm, *dirIn, std::forward<Args>(a)...);
        else o = new CUMDAT(onm, std::forward<Args>(a)...);
        cumDat.emplace(onm, o);
    }

    /// construct or restore non-cumulative by name
    template<class T, typename... Args>
    void registerWithName(T*& o, const string& onm, Args&&... a) {
        o = tryLoad<T>(onm);
        if(!o) addObject(o = new T(std::forward<Args>(a)...), onm);
    }

    /// get core histogram by name
    TH1* getSavedHist(const string& hname);
    /// get saved histogram by name, const version
    const TH1* getSavedHist(const string& hname) const;
    /// get cumulative data by name, const
    const CumulativeData* getCumulative(const string& cname) const;
    /// zero out all saved histograms
    virtual void zeroSavedHists();
    /// scale all saved histograms by a factor
    virtual void scaleData(double s);
    /// divide all (scaled) distributions by run time; should only be done once!
    virtual void normalize_runtime();
    /// check whether normalize_to_runtime has been performed
    bool isNormalized();
    /// helper to extract normalization (0 if not normalized)
    static double extract_norm(TFile& f);

    /// get total run timing
    virtual double getRuntime() const { return runTimes->GetTotal(); }
    /// add histograms, cumulatives from another SegmentSaver of the same type
    virtual void addSegment(const SegmentSaver& S, double sc = 1.);
    /// background subtract
    virtual void BGSubtract(SegmentSaver& BG) { BGData = &BG; addSegment(BG,-1.0); }
    /// check if this is equivalent layout to another SegmentSaver
    virtual bool isEquivalent(const SegmentSaver& S, bool throwit = false) const;
    /// statistical test of histogram similarity
    virtual map<string,float> compareKolmogorov(const SegmentSaver& S) const;

    bool ignoreMissingHistos = true;    ///< whether to quietly ignore missing histograms in input file
    TFile* fIn = nullptr;               ///< input file to read in histograms from
    TDirectory* dirIn = nullptr;        ///< particular sub-directory for reading histograms
    TVectorD* normalization = nullptr;  ///< normalization information; meaning defined in subclasses
    SegmentSaver* BGData = nullptr;     ///< optional subtracted background

    /// handle datastream signals
    void signal(datastream_signal_t s) override;

    double tSetup = 0;          ///< performance profiling: time to run constructor and initialize()
    double tProcess = 0;        ///< permormance profiling: time to process data
    double tCalc = 0;           ///< performance profiling: time for calculateResults
    double tPlot = 0;           ///< performance profiling: time for makePlots
    double order = 0;           ///< run sort ordering number


    // ----- Subclass me! ----- //
    /// optional mid-processing status check calculations/results/plots
    virtual void checkStatus() { }
    /// additional normalization on all histograms after normalize_runtime (e.g. conversion to differential rates); should only be done once!
    virtual void normalize() { printf(TERMFG_BLUE "\n--------- Normalizing '%s'... ----------" TERMSGR_RESET "\n\n", path.c_str()); }
    /// virtual routine for generating calculated hists
    virtual void calculateResults() { printf(TERMFG_BLUE "\n--------- '%s' calculating results... ----------" TERMSGR_RESET "\n\n", path.c_str()); }
    /// virtual routine for generating output plots
    virtual void makePlots() { printf(TERMFG_BLUE "\n--------- '%s' outputting plots... ----------" TERMSGR_RESET "\n\n", path.c_str()); }
    /// virtual routine for comparing to other analyzers (of this type or nullptr; meaning implementation-dependent)
    virtual void compare(const vector<SegmentSaver*>&) { }
    /// virtual routine to calculate incremental changes from preceding timestep
    virtual void checkpoint(const SegmentSaver&) { }


protected:

    /// attempt to load named object from file, registering and returning if successful
    TObject* _tryLoad(const string& oname);
    /// auto-recasting version
    template<class T>
    T* tryLoad(const string& oname) {
        auto o = _tryLoad(oname);
        if(!o) return nullptr;
        auto oo = dynamic_cast<T*>(o);
        if(!oo) throw std::runtime_error("Mismatched object type for "+oname);
        return oo;
    }

    /// clone from template or restore from file a saved TH1-derived class
    TH1* _registerSavedClone(const string& hname, const TH1& hTemplate);

    map<string, TH1*> saveHists;                            ///< saved cumulative histograms
    set<TH1*> doNotScale;                                   ///< saveHists items not to rescale
    map<string, CumulativeData*> cumDat;                    ///< additional cumulative types
    TCumulativeMap<string,Double_t>* runTimes = nullptr;    ///< run times for each input file
    TCumulativeMap<string,Double_t>* liveTimes = nullptr;   ///< optional separate per-object normalization livetimes
    map<string, string> xmeta;                              ///< extra metadata
};

#endif
