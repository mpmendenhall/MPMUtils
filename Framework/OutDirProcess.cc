/// \file OutDirProcess.cc

#include "OutDirProcess.hh"
#include <TFile.h>

void OutDirProcess::start_data(DataFrame& F) {
    // search up chain to find any preceding OutDirProcess
    OutDirProcess* P = nullptr;
    auto pp = parent;
    while(pp && !P) {
        P = dynamic_cast<OutDirProcess*>(pp);
        pp = pp->getParent();
    }

    // create subdirectory to parent
    if(P && !myDir && P->myDir) {
        myDir = P->myDir->mkdir(name.c_str());
        myDir->cd();
    }

    ConfigProcess::start_data(F);
    if(myDir) myDir->cd();
}

void OutDirProcess::end_data(DataFrame& F) {
    ConfigProcess::end_data(F);

    if(myDir) myDir->cd();
    for(auto o: writeObjs) {
        o.second->Write(o.first.c_str());
        delete o.second;
    }
    writeObjs.clear();

    auto f = dynamic_cast<TFile*>(myDir);
    if(f) { f->Close(); delete f; }
    myDir = nullptr;
}

template<>
TObject* OutDirProcess::addOutput(TObject* o, string n) {
    if(!o) return o;
    if(!n.size()) {
        auto oo = dynamic_cast<TNamed*>(o);
        if(!oo) throw; // needs a name specified if not a TNamed!
        n = oo->GetName();
    }
    writeObjs.emplace_back(n,o);
    return o;
}
