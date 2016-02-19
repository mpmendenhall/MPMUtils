/// \file SegmentSaver.hh Mechanism for loading pre-defined histograms from file
/* 
 * SegmentSaver.hh, part of the MPMUtils package.
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

#ifndef SEGMENTSAVER_HH
#define SEGMENTSAVER_HH

#include "OutputManager.hh"
#include <TH1.h>
#include <TVectorT.h>
#include <TObjString.h>
#include <TFile.h>
#include <map>
#include <string>
#include <cassert>

using std::map;
using std::string;

/// class for saving, retrieving, and summing histograms from file
class SegmentSaver: public OutputManager {
public:
    /// constructor, optionally with input filename
    SegmentSaver(OutputManager* pnt, const string& nm = "SegmentSaver", const string& inflName = "");
    /// destructor
    virtual ~SegmentSaver();
    /// get location of this analyzer's input file
    const string& getInflName() const { return inflname; }
    /// get age of analyzer's input file
    double getInflAge() const { return inflAge; }
    
    /// generate or restore from file a saved TH1F histogram
    TH1* registerSavedHist(const string& hname, const string& title,unsigned int nbins, float xmin, float xmax);
    /// generate or restore from file a saved TH2F histogram
    TH2* registerSavedHist2(const string& hname, const string& title,unsigned int nbinsx, float xmin, float xmax, float nbinsy, float ymin, float ymax);
    /// generate or restore from file a saved histogram from a template
    TH1* registerSavedHist(const string& hname, const TH1& hTemplate);
    /// display list of saved histograms
    void displaySavedHists() const;

    /// generate or restore from file a named TVectorD
    TVectorD* registerNamedVector(const string& vname, size_t nels = 0);
    /// generate or restore from file a named attribute string
    TObjString* registerAttrString(const string& nm, const string& val);
    /// clone or restore from file a template TObject
    TObject* registerObject(const string& onm, const TObject& oTemplate);
    
    /// get core histogram by name
    TH1* getSavedHist(const string& hname);
    /// get core histogram by name, const version
    const TH1* getSavedHist(const string& hname) const;
    /// get full histograms listing
    const map<string,TH1*>& getHists() const { return saveHists; }
    /// zero out all saved histograms
    virtual void zeroSavedHists();
    /// scale all saved histograms by a factor
    virtual void scaleData(double s);
    /// perform normalization on all histograms (e.g. conversion to differential rates); should only be done once!
    virtual void normalize() { }
    
    /// add histograms from another SegmentSaver of the same type
    virtual void addSegment(const SegmentSaver& S);
    /// check if this is equivalent layout to another SegmentSaver
    virtual bool isEquivalent(const SegmentSaver& S, bool throwit = false) const;
    /// load and add a list of segment files; return number loaded
    virtual size_t addFiles(const vector<string>& inflnames);
    
    bool ignoreMissingHistos;   ///< whether to quietly ignore missing histograms in input file
    
    
    // ----- Subclass me! ----- //
    
    /// create a new instance of this object(nm,inflname) (cloning self settings) for given directory
    virtual SegmentSaver* makeAnalyzer(const string&, const string&) { assert(false); return NULL; } 
    /// virtual routine for generating output plots
    virtual void makePlots() {}
    /// virtual routine for generating calculated hists
    virtual void calculateResults() { isCalculated = true; }
    /// virtual routine for comparing to other analyzers (of this type or NULL; meaning implementation-dependent)
    virtual void compare(const vector<SegmentSaver*>&) { }
    
    TFile* fIn;                 ///< input file to read in histograms from
    string inflname;            ///< where to look for input file
    bool isCalculated;          ///< flag for whether calculation step has been completed
    
protected:
    
    /// attempt to load named object from file, registering and returning if successful
    TObject* tryLoad(const string& oname);
    
    map<string,TH1*> saveHists; ///< saved histograms
    double inflAge;             ///< age of input file [s]; 0 for brand-new files
};

/// utility function to remove color axis data, to force re-draw with current dimensions
void resetZaxis(TH1* o);

#endif
