/// @file

#include "ROOTLoader.hh"
#include <TObjString.h>

const string& ROOTLoader::getMeta(const string& k) {
    auto it = xmeta.find(k);
    if(it != xmeta.end()) return it->second;

    /*
    TObject* o = nullptr;
    if(dirIn) dirIn->GetObject(("meta/"+k).c_str(), o);
    auto oo = dynamic_cast<TObjString*>(o);
    const auto& s = (xmeta[k] = oo? oo->GetString() : "");
    delete o;
    return s;
    */
    // TODO
    throw std::logic_error("Implement me!");
}

TH1* ROOTLoader::_registerSavedClone(const string& hname, const TH1& hTemplate) {
    auto h = tryLoad<TH1>(hname);
    if(!h) {
        h = static_cast<TH1*>(hTemplate.Clone(hname.c_str()));
        addObject(h);
        h->Reset();
    }
    return h;
}


//------------------------------------------------


TObject* TFileROOTLoader::_tryLoad(const string& oname) {
    if(!dirIn) return nullptr;

    TObject* o = nullptr;
    dirIn->GetObject(oname.c_str(), o);
    return o;
}


void TFileROOTLoader::setInput(const string& fname) {
    if(dirIn) delete dirIn;
    if(fIn) delete fIn;
    fIn = (fname.size())?(new TFile(fname.c_str(), "READ")) : nullptr;
    dirIn = fIn? fIn->GetDirectory("") : nullptr;
}
