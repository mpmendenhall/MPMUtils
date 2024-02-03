/// @file OutputManager.hh Organize output into parallel hierarchies of filesystem and ROOT TFile directories
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
    ~OutputManager() { endPrintMulti(); if(!parent) delete rootDir; }

    /// print current canvas; return filename printed
    virtual string printCanvas(const string& fname, TPad* P = nullptr, string suffix = "", const string& xsfx = "");
    /// precede each item of a group of prints to the same named file
    void printMulti(const string& fname, const string& suffix = "", TPad* P = nullptr);
    /// complete grouped set of print commands (empty string to close all open printsets)
    void endPrintMulti(const string& fname = "");

    /// print array/vector of several objects to same file
    template<typename ARRAY>
    void printTogether(const ARRAY& itms, const string& fname, const string& dopt = "", const string& suffix="") {
        for(auto h: itms) {
            h->Draw(dopt.c_str());
            printMulti(fname, suffix);
        }
        endPrintMulti(fname);
    }
    /// print map of several objects to same file
    template<typename K, typename V>
    void printTogether(const map<K,V>& itms, const string& fname, const string& dopt = "", const string& suffix="") {
        for(auto& kv: itms) {
            kv.second->Draw(dopt.c_str());
            printMulti(fname, suffix);
        }
        endPrintMulti(fname);
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
    /// reset canvas defaults

    /// default canvas dimensionssssss
    static void setup_default_pad(TVirtualPad* p = nullptr);

protected:
    string printsfx = ".pdf";           ///< printCanvas default suffix (file type)
    TDirectory* rootDir = nullptr;      ///< ROOT objects output directory

    /// Specification for a collection of prints to same file
    struct s_printset {
        int n = 0;  ///< number already printed
        string sfx; ///< printing suffix
    };
    map<string, s_printset> psets;      ///< currently open print sets by file name
};

#endif
