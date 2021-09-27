/// \file KTAccumJob.cc
// -- Michael P. Mendenhall, LLNL 2019

#include "KTAccumJob.hh"

void KTAccumJobComm::endJob(BinaryIO& B) {
    if(!combos.size()) {
        for(auto& kv: kt) {
            if(kv.first.substr(0,7) != "Combine") continue;
            combos.push_back(kv.second->Get<string>());
            auto cd = kt.FindKey(combos.back());
            if(!cd) throw std::runtime_error("Missing key for combining '" + combos.back() + "'");

            objs.push_back(nullptr);
            auto tp = cd->What();
            if(tp == kMESS_OBJECT) {
                objs.back() = cd->GetROOT<TH1>();
                if(objs.back()) objs.back()->Reset();
            } else cd->clearV();
        }
    }

    for(size_t i=0; i<combos.size(); i++) {
        auto cd = kt.FindKey(combos[i]);
        auto kd = B.receive<KeyData*>();
        if(!kd) throw std::logic_error("Failed to receive combining data  '" + combos[i] + "'");

        auto tp = cd->What();
        if(tp != kd->What()) throw std::logic_error("Mismatched types for combining '" + combos[i] + "'");

        if(tp == kMESS_OBJECT) {
            auto h = kd->GetROOT<TH1>();
            assert(h && objs[i]);
            objs[i]->Add(h);
            delete h;
        } else *cd += *kd;
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

void KTAccumJobComm::launchAccumulate(int uid) {
    vector<JobSpec> vJS;
    int nsamp = MultiJobControl::JC->nChunk();
    kt.Get("NSamples", nsamp);
    splitJobs(vJS, MultiJobControl::JC->nChunk(), nsamp, workerType(), uid);
    for(auto& j: vJS) MultiJobControl::JC->submitJob(j);
}

///////////////////////////////////////////////

REGISTER_FACTORYOBJECT(KTAccumJob, JobWorker)

void KTAccumJob::run(const JobSpec& J, BinaryIO& B) {
    JS = J;
    B.receive(kt);
    runAccum();
    MultiJobWorker::JW->signalDone();
    returnCombined(B);
}

void KTAccumJob::returnCombined(BinaryIO& B) {
    for(auto& kv: kt) {
        if(kv.first.substr(0,7) != "Combine") continue;
        auto c = kv.second->Get<string>();
        auto kd = kt.FindKey(c);
        if(!kd) throw std::runtime_error(("Missing return value for combine '"+c+"'").c_str());
        B.send(*kd);
    }
}

