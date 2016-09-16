/// \file TChainScanner.hh Utility for scanning data spread over many .root input files
/*
 * TChainScanner.hh, part of the MPMUtils package.
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

#ifndef TCHAINSCANNER_HH
#define TCHAINSCANNER_HH 1

#include <TChain.h>
#include <string>
#include <vector>

using std::string;
using std::vector;

/// class for assembling and scanning a TChain
class TChainScanner {
public:
    /// constructor
    TChainScanner(const string& treeName);
    /// destructor
    virtual ~TChainScanner() { delete Tch; }

    /// add a file to the TChain
    virtual int addFile(const string& filename);

    /// start a "speed scan," possibly at a random entry number
    virtual void startScan(bool startRandom = false);
    /// jump scanner to specified event
    virtual void gotoEvent(unsigned int e);
    /// load identified "speed scan" point
    virtual void speedload(unsigned int e);
    /// subclass call when new TTree loaded
    virtual void nextTreeLoaded() { }
    /// load next "speed scan" point
    virtual bool nextPoint();
    /// get current speed scan point
    unsigned int getCurrentEvent() const { return currentEvent; }
    /// load data for given event number
    virtual void getEvent(unsigned int e) { Tch->GetEvent(e); }
    /// get named branch address
    TChain* getChain() { return Tch; }
    /// get branch
    TBranch* getBranch(const char* bname) { return Tch->GetBranch(bname); }
    /// get local event number
    unsigned int getLocal(unsigned int e) { return Tch->LoadTree(e); }
    /// get number of files
    virtual unsigned int getnFiles() const { return nFiles; }

    UInt_t nEvents;                     ///< number of events in current TChain

    /// over-write this in subclass to automaticlly set readout points on first loaded file
    virtual void setReadpoints(TTree*) = 0;

protected:

    /// "string friendly" SetBranchAddress
    void SetBranchAddress(TTree* T, const string& bname, void* bdata);

    vector<unsigned int> nnEvents; ///< number of events in each loaded TChain;
    unsigned int nFiles;                ///< get number of loaded files
    bool noEmpty;                       ///< whether to abort on attempt to load empty files

    TChain* Tch;                        ///< TChain of relevant runs
    unsigned int currentEvent;          ///< event number of current event in chain
    unsigned int noffset;               ///< offset of current event relative to currently loaded tree
    unsigned int nLocalEvents;          ///< number of events in currently loaded tree
};

#endif
