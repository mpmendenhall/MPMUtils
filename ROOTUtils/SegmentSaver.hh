/// \file SegmentSaver.hh Mechanism for loading pre-defined histograms from file
// -- Michael P. Mendenhall, LLNL 2020

#ifndef SEGMENTSAVER_HH
#define SEGMENTSAVER_HH

#include "OutputManager.hh"
#include "TCumulative.hh"
#include "SignalSink.hh"
#include <TVectorT.h>
#include <TFile.h>
#include <stdexcept>
#include <set>
using std::set;

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
        if(saveHists.find(hname) != saveHists.end()) throw std::logic_error("Duplicate name '"+hname+"'"); // don't duplicate names!
        if(o) throw std::logic_error("Registration of '" + path + "/" + hname + "' would overwrite non-null pointer");
        o = tryLoad<T>(hname);
        if(!o) o = addObject(new T(hname.c_str(), std::forward<Args>(a)...));
        saveHists.emplace(hname, o);
    }

    /// clone from template or restore from file a saved TH1-derived class
    template<class T, class U>
    void registerSavedClone(T*& h, const string& hname, const U& hTemplate) {
        if(saveHists.find(hname) != saveHists.end()) throw std::logic_error("Duplicate name '"+hname+"'"); // don't duplicate names!
        if(h) throw std::logic_error("Registration of '" + path + "/" + hname + "' would overwrite non-null pointer");
        h = tryLoad<U>(hname);
        if(h) resetZaxis(h);
        else {
            h = addObject((U*)hTemplate.Clone(hname.c_str()));
            h->Reset();
        }
        saveHists.emplace(hname, h);
    }

    /// clone or restore from file a cumulative object
    TCumulative* _registerCumulative(const string& onm, const TCumulative& cTemplate);
    /// register cumulative with useful type return
    template<class T>
    T* registerCumulative(const string& onm, const T& cTemplate) {
        return static_cast<T*>(_registerCumulative(onm, cTemplate));
    }
    /// construct or retrieve named cumulative from file
    template<class CUMDAT, typename... Args>
    void registerAccumulable(CUMDAT*& o, const string& onm, Args&&... a) {
        if(cumDat.count(onm)) throw std::runtime_error("Duplicate cumulative name '" + onm + "'");
        if(o) throw std::logic_error("Registration of '" + path + "/" + onm + "' would overwrite non-null pointer");
        if(dirIn) o = new CUMDAT(onm, *dirIn, std::forward<Args>(a)...);
        else o = new CUMDAT(onm, std::forward<Args>(a)...);
        cumDat[onm] = o;
    }

    /// construct or restore non-cumulative by name
    template<class T, typename... Args>
    void registerWithName(T*& o, const string& onm, Args&&... a) {
        o = tryLoad<T>(onm);
        if(!o) addWithName(o = new T(std::forward<Args>(a)...), onm);
    }
    /// copy from template or restore non-cumulative by name
    template<class T>
    T* registerWithName(const string& onm, const T& oTemplate) {
        auto o = tryLoad<T>(onm);
        return o? o : addWithName(oTemplate.Clone(onm.c_str()), onm);
    }

    /// get core histogram by name
    TH1* getSavedHist(const string& hname);
    /// get saved histogram by name, const version
    const TH1* getSavedHist(const string& hname) const;
    /// get cumulative data by name, const
    const CumulativeData* getCumulative(const string& cname) const;
    /// get cumulative data by name, const
    const TCumulative* getTCumulative(const string& cname) const;
    /// get full histograms listing
    const map<string,TH1*>& getHists() const { return saveHists; }
    /// zero out all saved histograms
    virtual void zeroSavedHists();
    /// scale all saved histograms by a factor
    virtual void scaleData(double s);
    /// check whether normalization has been performed
    bool isNormalized();

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

    double tSetup = 0;          ///< performance profiling: time to run constructor
    double tProcess = 0;        ///< permormance profiling: time to process data
    double tCalc = 0;           ///< performance profiling: time for calculateResults
    double tPlot = 0;           ///< performance profiling: time for makePlots
    double order = 0;           ///< run sort ordering number


    // ----- Subclass me! ----- //
    /// optional mid-processing status check calculations/results/plots
    virtual void checkStatus() { }
    /// perform normalization on all histograms (e.g. conversion to differential rates); should only be done once!
    virtual void normalize() { if(!isNormalized()) { normalization->ResizeTo(1); (*normalization)(0) = 1; }  }
    /// virtual routine for generating calculated hists
    virtual void calculateResults() { }
    /// virtual routine for generating output plots
    virtual void makePlots() {}
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

    map<string,TH1*> saveHists;         ///< saved cumulative histograms
    set<void*> doNotScale;              ///< items not to rescale
    map<string, CumulativeData*> cumDat;    ///< non-TObject cumulative types
    map<string, TCumulative*> tCumDat;      ///< non-TH1-derived cumulative datatypes
    map<string, string> xmeta;          ///< extra metadata
};

/// utility function to remove color axis data, to force re-draw with current dimensions
void resetZaxis(TH1* o);

#endif
