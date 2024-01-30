/// \file JobState.cc
// -- Michael P. Mendenhall, LLNL 2023

#include "JobState.hh"
#include "DiskBIO.hh"
#include "PathUtils.hh"

string JobState::stateDir = "";

string JobState::sdataFile(const string& h) const {
    if(!stateDir.size()) return "";
    std::stringstream fn;
    fn << stateDir << "/SavedState_" << h << ".dat";
    return fn.str();
}

bool JobState::checkState(const string& h) {
    lastReq[h] = nReq++;
    if(stateData.count(h)) return true;
    if(!stateDir.size()) return false;

    auto f = sdataFile(h);
    FDBinaryReader b(f);
    if(!b.inIsOpen()) return false;
    //if(verbose > 3) printf("Loading persisted data from '%s'\n", f.c_str());
    b.receive(stateData[h]);
    return true;
}

void JobState::clearState(const string& h) {
    stateData.erase(h);
    lastReq.erase(h);
    if(stateDir.size()) syscmd("rm -f " + sdataFile(h));
}

void JobState::persistState(const string& h) {
    auto it = stateData.find(h);
    if(stateDir.size() && it != stateData.end()) {
        auto f = sdataFile(h);
        syscmd("mkdir -p " + stateDir);
        //if(verbose > 3) printf("Persisting state data to '%s'\n", f.c_str());
        {
            FDBinaryWriter b(f+"_tmp");
            b.send(it->second);
        }
        syscmd("mv " + f+"_tmp" + " " + f);
    }

    // purge excessive storage
    if(stateData.size() > 1000) {
        vector<string> vold;
        for(auto& kv: stateData) if(lastReq[kv.first] < nReq-500) vold.push_back(kv.first);
        for(auto hh: vold) if(hh != h) stateData.erase(h);
    }
}
