/// @file ROOTLoader.hh Interface for loading/storing ROOT objects from arbitrary backend

#ifndef ROOTLOADER_HH
#define ROOTLOADER_HH

#include "TObjCollector.hh"
#include <TH1.h>
#include <TFile.h>
#include <stdexcept>

/// Abstract interface for loading/storing ROOT objects
class ROOTLoader: public TObjCollector {
public:
    /// construct or retrieve TObject(Args...) by name
    template<class T, typename... Args>
    void registerWithName(T*& o, const string& onm, Args&&... a) {
        if(o) throw std::logic_error("Registration of '" + onm + "' would overwrite non-null pointer");
        o = tryLoad<T>(onm);
        if(!o) addObject(o = new T(std::forward<Args>(a)...), onm);
    }

    /// construct or retrieve saved TObject(Name, Args...)
    template<class T, typename... Args>
    void registerSaved(T*& o, const string& hname, Args&&... a) {
        registerWithName(o, hname, hname.c_str(), std::forward<Args>(a)...);
    }

    /// clone from template or restore from file a saved TH1-derived class
    template<class T, class U>
    typename std::enable_if<std::is_base_of<TH1, U>::value>::type registerSavedClone(T*& h, const string& hname, const U& hTemplate) {
        if(h) throw std::logic_error("Registration of '" + hname + "' would overwrite non-null pointer");
        h = static_cast<T*>(_registerSavedClone(hname, hTemplate));
    }

    /// get metadata string
    const string& getMeta(const string& k);
    /// set metadata string
    void setMeta(const string& k, const string& v) { xmeta[k] = v; }

    bool ignoreMissingObjects = true;   ///< whether to quietly ignore missing objects

    /// whether backend input is available
    virtual bool hasInput() const { return false; }

protected:
    /// Attempt to load object from underlying data source --- subclass me!
    virtual TObject* _tryLoad(const string&) { return nullptr; }

    /// auto-recasting version, with object save
    template<class T>
    T* tryLoad(const string& oname) {
        if(!hasInput()) return nullptr;

        auto o = _tryLoad(oname);
        if(!o) {
            if(ignoreMissingObjects) printf("Warning: missing object '%s'\n", oname.c_str());
            else throw std::runtime_error("Missing object '"+oname+"'");
            return nullptr;
        }
        auto oo = dynamic_cast<T*>(o);
        if(!oo) {
            delete o;
            throw std::runtime_error("Mismatched object type for "+oname);
        }
        addObject(o, oname);
        return oo;
    }

    /// clone from template or restore from file a saved TH1-derived class
    TH1* _registerSavedClone(const string& hname, const TH1& hTemplate);

    map<string, string> xmeta;      ///< extra metadata
};



/// TFile-based ROOTLoader
class TFileROOTLoader: public ROOTLoader {
public:
    /// Constructor
    explicit TFileROOTLoader(const string& inflname = "") { setInput(inflname); }
    /// Destructor
    ~TFileROOTLoader() { delete fIn; }

    /// set input file
    void setInput(const string& fname);

    /// whether backend input is available
    bool hasInput() const override { return dirIn; }

protected:
    /// Attempt to load object from underlying data source
    TObject* _tryLoad(const string& onm) override;

    TFile* fIn = nullptr;           ///< input file to read in objects
    TDirectory* dirIn = nullptr;    ///< particular sub-directory for reading objects
};

#endif
