/// @file SegmentSaver.cc

#include "SegmentSaver.hh"
#include "PathUtils.hh"
#include <stdexcept>
#include <TString.h>
#include <TObjString.h>

SegmentSaver::SegmentSaver(OutputManager* pnt, const string& _path, const string& inflname):
OutputManager(_path, pnt) {
    // open file to load existing data
    fIn = (inflname.size())?(new TFile(inflname.c_str(),"READ")) : nullptr;
    if(fIn) {
        dirIn = fIn->GetDirectory("");
        printf("Loading data from %s [%.1f hours old]...\n", inflname.c_str(), fileAge(inflname)/3600.);
    } else { // try sub-directory of parent if file not specified
        auto PSS = dynamic_cast<SegmentSaver*>(parent);
        if(PSS && PSS->dirIn) dirIn = PSS->dirIn->GetDirectory(path.c_str());
    }

    registerWithName(normalization, "normalization", 0);

    registerTCumulative(runTimes, "runTimes");
    runTimes->scalable = false;

    registerTCumulative(liveTimes, "liveTimes");
    liveTimes->scalable = false;
}

SegmentSaver::~SegmentSaver() {
    if(fIn) {
        fIn->Close();
        delete fIn;
    }
    //for(auto& kv: cumDat) delete kv.second;
}

void SegmentSaver::signal(datastream_signal_t s) {
    if(s >= DATASTREAM_END)
        for(auto& kv: cumDat) kv.second->endFill();
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


void SegmentSaver::normalize_runtime() {
    if(isNormalized()) throw std::logic_error("Normalization already applied");

    double rt = getRuntime();
    normalization->ResizeTo(1);
    (*normalization)(0) = rt;

    if(rt) printf("Normalizing to %g seconds runtime\n", rt);
    else {
        printf(TERMFG_RED "\nWARNING: zero runtime specified, normalization skipped" TERMSGR_RESET "\n\n");
        return;
    }

    for(auto& kv: saveHists) {
        if(doNotScale.count(kv.second)) continue;
        if(!kv.second->GetSumw2()) kv.second->Sumw2();
        auto it = liveTimes->find(kv.first);
        if(it == liveTimes->end()) kv.second->Scale(1./rt);
        else kv.second->Scale(1./it->second);
    }
    for(auto& kv: cumDat) {
        if(!kv.second->scalable) continue;
        auto it = liveTimes->find(kv.first);
        if(it == liveTimes->end()) kv.second->Scale(1./rt);
        else kv.second->Scale(1./it->second);
    }
}

double SegmentSaver::extract_norm(TFile& f) {
    auto norm = dynamic_cast<TVectorD*>(f.Get("normalization"));
    if(!norm) throw std::logic_error("run normalization undefined");
    auto n = norm->GetNrows()? (*norm)[0] : 0.;
    delete norm;
    return n;
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

TH1* SegmentSaver::_registerSavedClone(const string& hname, const TH1& hTemplate) {
    if(saveHists.count(hname)) throw std::logic_error("Duplicate name '"+hname+"'"); // don't duplicate names!
    auto h = tryLoad<TH1>(hname);
    if(h) resetZaxis(h);
    else {
        h = static_cast<TH1*>(hTemplate.Clone(hname.c_str()));
        addObject(h);
        h->Reset();
    }
    saveHists.emplace(hname, h);
    if(h->ClassName() == TString("TProfile") || h->ClassName() == TString("TProfile2D")) doNotScale.insert(h);
    return h;
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

const CumulativeData* SegmentSaver::getCumulative(const string& cname) const {
    auto it = cumDat.find(cname);
    if(it == cumDat.end()) throw std::runtime_error("Missing Cumulative '"+cname+"'");
    return it->second;
}

void SegmentSaver::zeroSavedHists() {
    for(auto& kv: saveHists) kv.second->Reset();
    for(auto& kv: cumDat) kv.second->ClearCumulative();
}

void SegmentSaver::scaleData(double s) {
    if(s == 1.) return;
    for(auto& kv: saveHists) {
        if(doNotScale.count(kv.second)) continue;
        if(!kv.second->GetSumw2()) kv.second->Sumw2();
        kv.second->Scale(s);
    }
    for(auto& kv: cumDat) if(kv.second->scalable) kv.second->Scale(s);
}

bool SegmentSaver::isEquivalent(const SegmentSaver& S, bool throwit) const {
    for(const auto& kv: saveHists) {
        if(!S.saveHists.count(kv.first)) {
            if(throwit) throw std::runtime_error("Mismatched histogram '"+kv.first+"' in '"+path+"'");
            return false;
        }
    }
    for(const auto& kv: cumDat) {
        if(!S.cumDat.count(kv.first)) {
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
    for(auto& kv: cumDat) {
        auto o = S.getCumulative(kv.first);
        if(o) kv.second->Add(*o, sc);
    }
}

void resetZaxis(TH1* o) {
    TObject* a = o->GetListOfFunctions()->FindObject("palette");
    if(a) o->GetListOfFunctions()->Remove(a);
}
