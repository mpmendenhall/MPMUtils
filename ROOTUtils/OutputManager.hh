/// \file OutputManager.hh Organize output into parallel hierarchies of filesystem and ROOT TFile directories
// Michael P. Mendenhall, LLNL 2020

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
    OutputManager(const string& bp, OutputManager* pnt = nullptr);
    /// Destructor
    ~OutputManager() { if(!parent) delete rootDir; }

    /// print current canvas; return filename printed
    virtual string printCanvas(const string& fname, const TPad* P = nullptr, string suffix="") const;
    /// set printCanvas suffix (filetype)
    virtual void setPrintSuffix(const string& sfx) { printsfx = sfx; }

    /// open output ROOT file
    void openROOT(OutputManager* base=nullptr);
    /// write output ROOT file (or directory within parent)
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
