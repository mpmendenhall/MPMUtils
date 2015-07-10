/// \file TChainScanner.cc
/* 
 * TChainScanner.cc, part of the MPMUtils package.
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

#include "TChainScanner.hh"
#include "SMExcept.hh"
#include <stdlib.h>
#include <time.h>

TChainScanner::TChainScanner(const string& treeName): nEvents(0), nFiles(0), noEmpty(false), Tch(new TChain(treeName.c_str())),
currentEvent(0), noffset(0), nLocalEvents(0) {
    Tch->SetMaxVirtualSize(10000000);
}


int TChainScanner::addFile(const string& filename) {
    unsigned int oldEvents = nEvents;
    int nfAdded = Tch->Add(filename.c_str(),0);
    if(!nfAdded) {
        SMExcept e("missingFiles");
        e.insert("fileName",filename);
        throw e;
    }
    nEvents = Tch->GetEntries();
    nnEvents.push_back(nEvents-oldEvents);
    if(!nnEvents.back() && noEmpty) {
        SMExcept e("noEventsInFile");
        e.insert("fileName",filename);
        e.insert("nFiles",nfAdded);
        throw e;
    }
    if(!nFiles)
        setReadpoints(Tch);
    nFiles+=nfAdded;
    return nfAdded;
}

void TChainScanner::gotoEvent(unsigned int e) {
    currentEvent = e;
    Tch->GetEvent(currentEvent);
    smassert(Tch->GetTree());
    nLocalEvents = noffset = 0;
}

void TChainScanner::startScan(bool startRandom) { 
    if(!nEvents) {
        printf("Starting scan with no data... ");
        fflush(stdout);
        return;
    }
    if(startRandom) {
        if(!currentEvent) {
            srand(time(NULL)); // random random seed
            gotoEvent(rand()%Tch->GetEntries());
            printf("Scan Starting at offset %i/%i: ",currentEvent,nEvents);
        } else {
            printf("Scan Continuing at offset %i/%i: ",currentEvent,nEvents);
        }
    } else {
        gotoEvent(0);
        currentEvent = -1;
        printf(">%i< ",nEvents);
    }
    fflush(stdout);
}

void TChainScanner::SetBranchAddress(TTree* T, const string& bname, void* bdata) {
    smassert(bdata);
    smassert(T);
    Int_t err = T->SetBranchAddress(bname.c_str(),bdata);
    if(err && err != TTree::kNoCheck) {
        SMExcept e("TTreeBranchError");
        e.insert("branchName", bname);
        e.insert("errCode",err);
        throw e;
    }
}

void TChainScanner::speedload(unsigned int e) {
    if(e < noffset || e-noffset >= nLocalEvents) {
        Tch->LoadTree(e);
        nLocalEvents = Tch->GetTree()->GetEntries();
        noffset = Tch->GetChainOffset();
        nextTreeLoaded();
    }
    Tch->GetTree()->GetEvent(e-noffset);
}

bool TChainScanner::nextPoint() {
    if(!nEvents) return false;
    ++currentEvent;
    if(currentEvent >= nEvents) {
        printf("\n");
        startScan();
        return false;
    }
    if(!(currentEvent%(nEvents/20))) {
        printf("*"); fflush(stdout);
    }
    speedload(currentEvent);
    return true;
}
