/// \file OutputManager.hh Organize output into parallel hierarchies of filesystem and ROOT TFile directories
// -- Michael P. Mendenhall, LLNL 2020

#ifndef OUTPUTMANAGER_HH
#define OUTPUTMANAGER_HH

#include "TObjCollector.hh"
#include <TCanvas.h>
#include <TPad.h>

/// Organize output into parallel hierarchies of filesystem and ROOT TFile directories
/*
 * OutputManager parent("path/to/parent");
 * OutputManager child("child", &parent);
 *
 * path/to/
 *         parent.root
 *              (parents's TObjects)
 *              child/
 *                  (child's TObjects)
 *         parent/
 *              (parent's outputs)
 *              child/
 *                    (child's outputs)
 *
*/
class OutputManager: public TObjCollector {
public:
    /// Constructor
    explicit OutputManager(const string& bp, OutputManager* pnt = nullptr);
    /// Destructor
    ~OutputManager() { if(!parent) delete rootDir; }

    /// print current canvas; return filename printed
    virtual string printCanvas(const string& fname, const TPad* P = nullptr, string suffix="") const;
    /// print array/vector of several objects to same file
    template<typename ARRAY>
    void printTogether(ARRAY& itms, const string& fname, const string& dopt = "", string suffix="") const {
        if(!suffix.size()) suffix = printsfx;
        if(itms.size() == 1) {
            (*itms.begin())->Draw(dopt.c_str());
            printCanvas(fname, nullptr, suffix);
            return;
        }
        for(auto h: itms) {
            h->Draw(dopt.c_str());
            printCanvas(fname, nullptr, suffix + (h == *itms.begin()? "(" : h == itms.back()? ")" : ""));
        }
    }
    /// print map of several objects to same file
    template<typename K, typename V>
    void printTogether(map<K,V>& itms, const string& fname, const string& dopt = "", string suffix="") const {
        if(!suffix.size()) suffix = printsfx;
        if(itms.size() == 1) {
            itms.begin()->second->Draw(dopt.c_str());
            printCanvas(fname, nullptr, suffix);
            return;
        }
        for(auto& kv: itms) {
            auto& h = kv.second;
            h->Draw(dopt.c_str());
            printCanvas(fname, nullptr, suffix + (&h == &itms.begin()->second? "(" : &h == &itms.rbegin()->second? ")" : ""));
        }
    }
    /// set printCanvas suffix (filetype)
    virtual void setPrintSuffix(const string& sfx) { printsfx = sfx; }

    /// write output ROOT file fullpath().root (or directory within parent)
    string writeROOT();

    TCanvas defaultCanvas;              ///< canvas for drawing plots
    OutputManager* parent = nullptr;    ///< parent output manager (provides starting subdirectories)
    string path;                        ///< output name/path (relative to parent if provided)
    static bool squelchAllPrinting;     ///< whether to cancel all printCanvas output

    /// ROOT output directory
    TDirectory* getRootOut();
    /// full output path
    string fullPath() const;

protected:
    string printsfx = ".pdf";           ///< printCanvas default suffix (file type)
    TDirectory* rootDir = nullptr;      ///< ROOT objects output directory
};

#endif
