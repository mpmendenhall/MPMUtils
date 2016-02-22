/// \file SegmentSaver.cc
/* 
 * SegmentSaver.cc, part of the MPMUtils package.
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

#include "SegmentSaver.hh"
#include "PathUtils.hh"
#include "SMExcept.hh"
#include <TString.h>

void resetZaxis(TH1* o) {
    TObject* a = o->GetListOfFunctions()->FindObject("palette");
    if(a) o->GetListOfFunctions()->Remove(a);
}

TObject* SegmentSaver::tryLoad(const string& oname) {
    if(!fIn) return NULL;
    TObject* o = NULL;
    fIn->GetObject(oname.c_str(),o);
    if(!o) {
        if(ignoreMissingHistos) {
            printf("Warning: missing object '%s' in '%s'\n",oname.c_str(),inflname.c_str());
        } else {
            SMExcept e("fileStructureMismatch");
            e.insert("fileName",inflname);
            e.insert("objectName",oname);
            throw(e);
        }
    } else {
        addWithName(o, oname);
    }
    return o;
}

TH1* SegmentSaver::registerSavedHist(const string& hname, const string& title,unsigned int nbins, float xmin, float xmax) {
    smassert(saveHists.find(hname)==saveHists.end(), "duplicate_name_"+hname); // don't duplicate names!
    TH1* h = dynamic_cast<TH1*>(tryLoad(hname));
    if(!h) h = registeredTH1F(hname,title,nbins,xmin,xmax);
    saveHists.emplace(hname,h);
    return h;
}

TH2* SegmentSaver::registerSavedHist2(const string& hname, const string& title,unsigned int nbinsx, float xmin, float xmax, float nbinsy, float ymin, float ymax) {
    smassert(saveHists.find(hname)==saveHists.end(), "duplicate_name_"+hname); // don't duplicate names!
    TH2* h = dynamic_cast<TH2*>(tryLoad(hname));
    if(h) resetZaxis(h);
    else h = registeredTH2F(hname,title,nbinsx,xmin,xmax,nbinsy,ymin,ymax);
    saveHists.emplace(hname,h);
    return h;
}

TH1* SegmentSaver::registerSavedHist(const string& hname, const TH1& hTemplate) {
    smassert(saveHists.find(hname)==saveHists.end(), "duplicate_name_"+hname); // don't duplicate names!
    TH1* h = dynamic_cast<TH1*>(tryLoad(hname));
    if(h) resetZaxis(h);
    else {
        h = (TH1*)addObject((TH1*)hTemplate.Clone(hname.c_str()));
        h->Reset();
    }
    saveHists.emplace(hname,h);
    return h;
}

TVectorD* SegmentSaver::registerNamedVector(const string& vname, size_t nels) {
    TVectorD* V = dynamic_cast<TVectorD*>(tryLoad(vname));
    return V? V : (TVectorD*)addWithName(new TVectorD(nels), vname);
}

TObjString* SegmentSaver::registerAttrString(const string& nm, const string& val) {
    TObjString* s = dynamic_cast<TObjString*>(tryLoad(nm));
    return s? s : (TObjString*)addWithName(new TObjString(val.c_str()), nm);
}

TObject* SegmentSaver::registerObject(const string& onm, const TObject& oTemplate) {
    TObject* o = tryLoad(onm);
    return o? o : addWithName(oTemplate.Clone(onm.c_str()), onm);
}

SegmentSaver::SegmentSaver(OutputManager* pnt, const string& nm, const string& inflName):
OutputManager(nm,pnt), ignoreMissingHistos(false), inflname(inflName), isCalculated(false), inflAge(0) {
    // open file to load existing data
    fIn = (inflname.size())?(new TFile(inflname.c_str(),"READ")):NULL;
    smassert(!fIn || !fIn->IsZombie(),"unreadable_file");
    if(fIn) {
        inflAge = fileAge(inflname);
        printf("Loading data from %s [%.1f hours old]...\n",inflname.c_str(),inflAge/3600.);
    }
}

SegmentSaver::~SegmentSaver() {
    if(fIn) {
        fIn->Close();
        delete fIn;
    }
}

TH1* SegmentSaver::getSavedHist(const string& hname) {
    auto it = saveHists.find(hname);
    smassert(it != saveHists.end());
    return it->second;
}

const TH1* SegmentSaver::getSavedHist(const string& hname) const {
    auto it = saveHists.find(hname);
    if(it == saveHists.end()) {
        SMExcept e("missing_histogram");
        e.insert("name", hname);
        throw e;
    }
    return it->second;
}

void SegmentSaver::zeroSavedHists() {
    for(auto& kv: saveHists) kv.second->Reset();
}

void SegmentSaver::scaleData(double s) {
    if(s==1.) return;
    for(auto& kv: saveHists) {
        if(kv.second->ClassName() != TString("TProfile")) {
            if(!kv.second->GetSumw2()) kv.second->Sumw2();
            kv.second->Scale(s);
        }
    }
}

bool SegmentSaver::isEquivalent(const SegmentSaver& S, bool throwit) const {
    for(auto& kv: saveHists) {
        if(!S.saveHists.count(kv.first)) {
            if(throwit) {
                SMExcept e("mismatched_histogram");
                e.insert("name", kv.first);
                throw e;
            }
            return false;
        }
    }
    return true;
}

void SegmentSaver::addSegment(const SegmentSaver& S) {
    isEquivalent(S, true);
    for(auto& kv: saveHists) kv.second->Add(S.getSavedHist(kv.first));        
}

size_t SegmentSaver::addFiles(const vector<string>& inflnames) {
    size_t nMerged = 0;
    for(auto f: inflnames) {
        if(!fileExists(f)) continue;
        SegmentSaver* subRA = makeAnalyzer("", f);
        addSegment(*subRA);
        delete subRA;
        nMerged++;
    }
    return nMerged;
}

void SegmentSaver::displaySavedHists() const {
    for(auto& kv: saveHists) printf("\t'%s'\n", kv.first.c_str());
}

