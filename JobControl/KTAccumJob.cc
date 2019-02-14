/// \file KTAccumJob.cc
// Michael P. Mendenhall, LLNL 2019

#include "KTAccumJob.hh"

void KTAccumJobComm::endJob(BinaryIO& B) {
    if(!combos.size()) {
        for(auto& kv: kt) {
            if(kv.first.substr(0,7) != "Combine") continue;
            combos.push_back(kv.second->Get<string>());
            auto cd = kt.FindKey(combos.back());
            if(!cd) exit(10);

            objs.push_back(nullptr);
            auto tp = cd->What();
            if(tp == kMESS_OBJECT) {
                objs.back() = cd->GetROOT<TH1>();
                if(objs.back()) objs.back()->Reset();
            } else if(tp == KeyData::kMESS_DOUBLE) cd->clear<double>();
        }
    }

    for(size_t i=0; i<combos.size(); i++) {
        auto cd = kt.FindKey(combos[i]);
        if(!cd) exit(22);
        auto kd = B.receive<KeyData*>();
        if(!kd) exit(23);

        auto tp = cd->What();
        if(tp != kd->What()) exit(24);


        if(tp == KeyData::kMESS_DOUBLE) cd->accumulate<double>(*kd);
        else if(tp == kMESS_OBJECT) {
            auto h = kd->GetROOT<TH1>();
            if(!h || !objs[i]) exit(25);
            objs[i]->Add(h);
            delete h;
        }
        delete kd;
    }
}

void KTAccumJobComm::gather() {
    for(size_t i=0; i<combos.size(); i++) {
        if(kt.FindKey(combos[i])->What() == kMESS_OBJECT) kt.Set(combos[i], *objs[i]);
        delete objs[i];
    }
    combos.clear();
    objs.clear();
}

void KTAccumJobComm::launchAccumulate(size_t wid, int uid) {
    vector<JobSpec> vJS;
    int nsamp = MultiJobControl::JC->nChunk();
    kt.Get("NSamples", nsamp);
    splitJobs(vJS, MultiJobControl::JC->nChunk(), nsamp, wid, uid);
    for(auto& j: vJS) MultiJobControl::JC->submitJob(j);
}

///////////////////////////////////////////////

REGISTER_FACTORYOBJECT(KTAccumJob)

void KTAccumJob::run(JobSpec J, BinaryIO& B) {
    JS = J;
    B.receive(kt);
    runAccum();
    MultiJobControl::JC->signalDone();
    returnCombined(B);
}

void KTAccumJob::returnCombined(BinaryIO& B) {
    for(auto& kv: kt) {
        if(kv.first.substr(0,7) != "Combine") continue;
        auto kd = kt.FindKey(kv.second->Get<string>());
        if(!kd) exit(11);
        B.send(*kd);
    }
}

