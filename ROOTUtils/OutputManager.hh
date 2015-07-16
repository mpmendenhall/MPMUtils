/// \file OutputManager.hh \brief Convenience class for bundling ROOT output into a directory
/* 
 * OutputManager.hh, part of the MPMUtils package.
 * Copyright (c) 2014 Michael P. Mendenhall
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */

#ifndef OUTPUTMANAGER_HH
#define OUTPUTMANAGER_HH

#include "TObjCollector.hh"
#include <TCanvas.h>
#include <TH1.h>
#include <TH1F.h>
#include <TH2F.h>

/// indicator for level of problem with analysis
enum WarningLevel {
    BENIGN_WARNING,     ///< just FYI for troubleshooting
    MODERATE_WARNING,   ///< might be a problem to look at
    SEVERE_WARNING,     ///< almost certainly something is very wrong
    FATAL_WARNING       ///< data is corrupted and cannot be analyzed
};

/// manages output directory for grouping related information; manages a canvas, output SMFile, output ROOT file, recursive subdirectories
class OutputManager: public TObjCollector {
public:
    /// constructor for top-level
    OutputManager(string nm, string bp);
    /// constructor for nested
    OutputManager(string nm, OutputManager* pnt);
    
    /// destructor
    virtual ~OutputManager() {
        if(writeRootOnDestruct) writeROOT();
        clearItems();
        if(rootOut) rootOut->Close();
        if(defaultCanvas && !parent) delete(defaultCanvas); 
    }
    
    /// generate a TH1F registered with this runs output objects list
    TH1F* registeredTH1F(string hname, string htitle, unsigned int nbins, float x0, float x1);
    /// generate a TH2F registered with this runs output objects list
    TH2F* registeredTH2F(string hname, string htitle, unsigned int nbinsx, float x0, float x1, unsigned int nbinsy, float y0, float y1);
    /// print current canvas
    virtual void printCanvas(string fname, string suffix=".pdf") const;
        
    /// open output ROOT file for writing (useful if output tree is being created and needs a home)
    void openOutfile();
    /// set whether to write ROOT output when destructed
    void setWriteRoot(bool w) { writeRootOnDestruct = w; }
    
    TFile* rootOut;             ///< ROOT file output
    TCanvas* defaultCanvas;     ///< canvas for drawing plots
    OutputManager* parent;      ///< parent output manager
    string basePath;            ///< general output path
    string plotPath;            ///< specific output path for plots
    string dataPath;            ///< specific output path for output data
    string rootPath;            ///< specific output path for ROOT files
    string name;                ///< name for this subsystem
    bool writeRootOnDestruct;   ///< whether to write ROOT file when destructed
    
    static bool squelchAllPrinting;     ///< whether to cancel all printCanvas output
    
protected:
    
    /// write output ROOT file; WARNING: THIS DELETES ALL REGISTERED ITEMS; do last if you reference these.
    void writeROOT();
};

#endif
