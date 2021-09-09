/// \file SegmentSaver.cc

#include "SegmentSaver.hh"
#include "PathUtils.hh"
#include <stdexcept>
#include <TString.h>
#include <TObjString.h>

SegmentSaver::SegmentSaver(OutputManager* pnt, const string& nm, const string& inflname):
OutputManager(nm, pnt) {
    // open file to load existing data
    fIn = (inflname.size())?(new TFile(inflname.c_str(),"READ")) : nullptr;
    if(fIn) {
        dirIn = fIn->GetDirectory("");
        printf("Loading data from %s [%.1f hours old]...\n", inflname.c_str(), fileAge(inflname)/3600.);
    } else { // try sub-directory of parent if file not specified
        auto PSS = dynamic_cast<SegmentSaver*>(parent);
        if(PSS && PSS->dirIn) dirIn = PSS->dirIn->GetDirectory(nm.c_str());
    }

    registerWithName(normalization, "normalization", 0);
}

SegmentSaver::~SegmentSaver() {
    if(fIn) {
        fIn->Close();
        delete fIn;
    }
    for(auto& kv: cumDat) delete kv.second;
}

const string& SegmentSaver::getMeta(const string& k) {
    auto it = xmeta.find(k);
    if(it != xmeta.end()) return it->second;

    TObject* o = nullptr;
    if(dirIn) dirIn->GetObject(("meta/"+k).c_str(), o);
    auto oo = dynamic_cast<TObjString*>(o);
    const auto& s = (xmeta[k] = oo? oo->GetString() : "");
    delete o;
    return s;
}

TDirectory* SegmentSaver::writeItems(TDirectory* d) {
    TObjString* s = nullptr;
    for(auto& kv: xmeta) registerWithName(s, "meta/"+kv.first, kv.second.data());

    d = OutputManager::writeItems(d);
    if(d) d->cd();
    for(auto& kv: cumDat) kv.second->Write();
    return d;
}

bool SegmentSaver::isNormalized() { return normalization->GetNrows(); }

void SegmentSaver::rename(const string& nm) {
    path = nm;
    if(!fIn) {
        auto PSS = dynamic_cast<SegmentSaver*>(parent);
        if(PSS && PSS->dirIn) dirIn = PSS->dirIn->GetDirectory(path.c_str());
    }
}

TObject* SegmentSaver::_tryLoad(const string& oname) {
    if(!dirIn) return nullptr;
    TObject* o = nullptr;
    dirIn->GetObject(oname.c_str(), o);
    if(!o) {
        if(ignoreMissingHistos) {
            printf("Warning: missing object '%s' in '%s'\n",
                   oname.c_str(), dirIn? dirIn->GetName() : fIn? fIn->GetName() : "Unknown");
        } else {
            throw std::runtime_error("File structure mismatch: missing '"+oname+"'");
        }
    } else {
        addWithName(o, oname);
    }
    return o;
}

TCumulative* SegmentSaver::_registerCumulative(const string& onm, const TCumulative& cTemplate) {
    auto c = tryLoad<TCumulative>(onm);
    if(!c) {
        c = static_cast<TCumulative*>(addObject((TNamed*)cTemplate.Clone(onm.c_str())));
        c->Clear();
    }
    tCumDat.emplace(onm,c);
    return c;
}

TH1* SegmentSaver::getSavedHist(const string& hname) {
    auto it = saveHists.find(hname);
    if(it == saveHists.end()) throw std::runtime_error("Missing histogram '"+hname+"'");
    return it->second;
}

const TH1* SegmentSaver::getSavedHist(const string& hname) const {
    auto it = saveHists.find(hname);
    if(it == saveHists.end()) throw std::runtime_error("Missing histogram '"+hname+"'");
    return it->second;
}

const TCumulative* SegmentSaver::getTCumulative(const string& cname) const {
    auto it = tCumDat.find(cname);
    if(it == tCumDat.end()) throw std::runtime_error("Missing TCumulative '"+cname+"'");
    return it->second;
}

const CumulativeData* SegmentSaver::getCumulative(const string& cname) const {
    auto it = cumDat.find(cname);
    if(it == cumDat.end()) throw std::runtime_error("Missing Cumulative '"+cname+"'");
    return it->second;
}

void SegmentSaver::zeroSavedHists() {
    for(auto& kv: saveHists) kv.second->Reset();
    for(auto& kv: cumDat) kv.second->Scale(0);
    for(auto& kv: tCumDat) kv.second->Clear();
}

void SegmentSaver::scaleData(double s) {
    if(s == 1.) return;
    for(auto& kv: saveHists) {
        if(doNotScale.count(kv.second)) continue;
        if(kv.second->ClassName() != TString("TProfile") && kv.second->ClassName() != TString("TProfile2D")) {
            if(!kv.second->GetSumw2()) kv.second->Sumw2();
            kv.second->Scale(s);
        }
    }
    for(auto& kv: cumDat) if(!doNotScale.count(kv.second)) kv.second->Scale(s);
    for(auto& kv: tCumDat) if(!doNotScale.count(kv.second)) kv.second->Scale(s);
}

bool SegmentSaver::isEquivalent(const SegmentSaver& S, bool throwit) const {
    for(auto& kv: saveHists) {
        if(!S.saveHists.count(kv.first)) {
            if(throwit) throw std::runtime_error("Mismatched histogram '"+kv.first+"' in '"+path+"'");
            return false;
        }
    }
    for(auto& kv: tCumDat) {
        if(!S.tCumDat.count(kv.first)) {
            if(throwit) throw std::runtime_error("Mismatched cumulative '"+kv.first+"' in '"+path+"'");
            return false;
        }
    }
    return true;
}

map<string,float> SegmentSaver::compareKolmogorov(const SegmentSaver& S) const {
    map<string,float> m;
    for(auto& kv: saveHists) {
        if(kv.second->GetEntries() < 100) continue;
        auto it = S.saveHists.find(kv.first);
        if(it == S.saveHists.end()) continue;
        m[kv.first] = kv.second->KolmogorovTest(it->second,"UO");
    }
    return m;
}

void SegmentSaver::addSegment(const SegmentSaver& S, double sc) {
    isEquivalent(S, true);
    for(auto& kv: saveHists) kv.second->Add(S.getSavedHist(kv.first), sc);
    for(auto& kv: tCumDat) {
        auto o = S.getTCumulative(kv.first);
        if(o) kv.second->Add(*o, sc);
    }
    for(auto& kv: cumDat) {
        auto o = S.getCumulative(kv.first);
        if(o) kv.second->Add(*o, sc);
    }
}

void resetZaxis(TH1* o) {
    TObject* a = o->GetListOfFunctions()->FindObject("palette");
    if(a) o->GetListOfFunctions()->Remove(a);
}
