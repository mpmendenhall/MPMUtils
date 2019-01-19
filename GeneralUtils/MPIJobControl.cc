/// \file MPIJobControl.cc
// Michael P. Mendenhall, LLNL 2019

#include "MPIJobControl.hh"
#include <iostream>
#include <TMessage.h>
#include <TH1.h>

int MPIJobControl::coresPerNode = 0;
int MPIJobControl::ntasks = 0;
int MPIJobControl::verbose = 0;
int MPIJobControl::rank = 0;
int MPIJobControl::parentRank = 0;
vector<int> MPIJobControl::childRanks;
char MPIJobControl::hostname[MPI_MAX_PROCESSOR_NAME];

void MPIJobControl::init(int argc, char **argv) {
    int status = MPI_Init(&argc, &argv);
    if(status != MPI_SUCCESS) {
        std::cout << "MPI Init Error." << endl;
        MPI_Abort(MPI_COMM_WORLD,status);
    }

    // Get information about task.
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &ntasks);
    int length = 0;
    MPI_Get_processor_name(hostname, &length);
    coresPerNode = atol(getenv("SLURM_CPUS_ON_NODE"));

    if(ntasks <= coresPerNode) { // local single-level distribution

        if(!rank) for(int i = 1; i < ntasks; i++) childRanks.push_back(i);

    } else { // multi-level distribution

        if(!rank) { // top level

            int numControllers = ntasks/coresPerNode;
            for(int i = 0; i < numControllers; i++) {
                int r = i*coresPerNode;
                childRanks.push_back(r? r : 1);
            }

        } else if(rank == 1 || rank % coresPerNode == 0) { // controller nodes

            int rankStart = rank + 1;
            int rankEnd = ((rank/coresPerNode)+1)*coresPerNode;
            for(int i=rankStart; i<rankEnd; i++) childRanks.push_back(i);
        }

        parentRank = childRanks.size()? 0 : (rank/coresPerNode)*coresPerNode;
        if(rank > 1 && parentRank == 0) parentRank = 1;
    }

    if(verbose) {
        std::cout << "Rank " << rank << " task of " << ntasks << " available on " << hostname;
        std::cout << " (" << coresPerNode << " cores) starting run.\n";
        std::cout << "\tParent: " << parentRank << "; children: <";
        for(auto r: childRanks) std::cout << " " << r;
        std::cout << " >\n";
    }

    if(rank) {
        if(childRanks.size()) runController();
        else runWorker();
    }
}

void MPIJobControl::finish() {
    // Send ending message to close worker process
    for(auto r: childRanks) mpiSend<int>(r,1);

    if(verbose > 1) printf(childRanks.size()? "Controller [%i] closing.\n" : "Worker [%i] closing.\n", rank);
    MPI_Finalize();
}

void MPIJobControl::accumulate(KeyTable& kt) {
    // distribute to child nodes
    if(verbose > 2 && childRanks.size()) printf("Node [%i] distributing jobs to %zu children.\n", rank, childRanks.size());
    for(auto r: childRanks) {
        mpiSend<int>(r, 0);  // continue command-processing loop
        mpiSend(r, kt);      // send simulation instructions
    }
    // run one job locally
    if(verbose > 2) printf("Node [%i] running one job locally.\n", rank);
    workerJob(kt);

    if(!childRanks.size()) {
        if(rank != parentRank) returnAccumulate(kt);
        return;
    }

    // load summing objects
    vector<TH1*> objs;
    vector<string> combos;
    for(auto& kv: kt) {
        if(kv.first.substr(0,7) != "Combine") continue;
        combos.push_back(kv.second->GetString());
        auto cd = kt.FindKey(combos.back());
        if(!cd) exit(1);

        objs.push_back(nullptr);
        if(cd->What() == kMESS_OBJECT) objs.back() = cd->GetROOT<TH1>();
    }
    auto nReturn = combos.size();

    // Receive and sum data from subnodes
    for(int r: childRanks) {
        for(size_t i=0; i<nReturn; i++) {
            auto cd = kt.FindKey(combos[i]);
            auto kd = mpiReceive<KeyData*>(r);

            auto tp = cd->What();
            if(tp == KeyData::kMESS_DOUBLE) cd->accumulate<double>(*kd);
            else if(tp == kMESS_OBJECT) {
                auto h = kd->GetROOT<TH1>();
                if(h) objs[i]->Add(h);
                delete h;
            }

            delete kd;
        }
    }

    for(size_t i=0; i<nReturn; i++) {
        if(kt.FindKey(combos[i])->What() == kMESS_OBJECT) kt.Set(combos[i], objs[i]);
        delete objs[i];
    }

    if(rank != parentRank) returnAccumulate(kt);
}

void MPIJobControl::returnAccumulate(KeyTable& kt) {
    if(verbose > 2) printf("Node [%i] returning results to [%i]...\n", rank, parentRank);
    for(auto& kv: kt) {
        if(kv.first.substr(0,7) != "Combine") continue;
        auto kd = kt.FindKey(kv.second->GetString());
        if(!kd) exit(1);
        mpiSend(parentRank, *kd);
    }
}

void MPIJobControl::runController() {
    while(mpiReceive<int>(parentRank) != 1) { // do jobs until break command ('1') received
        // Receive KeyTable.
        KeyTable kt = mpiReceive<KeyTable>(parentRank);
        // Farm out to individual worker nodes
        accumulate(kt); // also returns to parent
    }
}

void MPIJobControl::runWorker() {
    while(mpiReceive<int>(parentRank) != 1) { // do jobs until break command ('1') received

        // Receive KeyTable with instructions for simulation
        KeyTable kt = mpiReceive<KeyTable>(MPIJobControl::parentRank);
        // Run the job
        workerJob(kt);
        // Return results.
        returnAccumulate(kt);
    }
}

////////////////////////////////////////////////
////////////////////////////////////////////////
////////////////////////////////////////////////

template<>
void mpiSend<string>(int rank, const string& s) {
    mpiSend<int>(rank, s.size());
    mpiSend(rank, (void*)s.data(), s.size());
}

template<>
string mpiReceive<string>(int rank) {
    int length = mpiReceive<int>(rank);
    string s(length, ' ');
    mpiReceive(rank, &s[0], length);
    return s;
}

template<>
void mpiSend<KeyData>(int rank, const KeyData& M) {
    mpiSend<UInt_t>(rank, M.BufferSize());
    mpiSend(rank, M.Buffer(), M.BufferSize());
}

template<>
KeyData* mpiReceive<KeyData*>(int rank) {
    auto s = mpiReceive<UInt_t>(rank);
    if(!s) return nullptr;

    auto buf = new char[s];
    mpiReceive(rank, buf, s);
    return new KeyData(buf, s);
}

template<>
void mpiSend<KeyTable>(int rank, const KeyTable& kt) {
    mpiSend<size_t>(rank, kt.size());
    for(auto& kv: kt) {
        mpiSend<string>(rank, kv.first);
        mpiSend(rank, *kv.second);
    }
}

template<>
KeyTable mpiReceive<KeyTable>(int rank) {
    KeyTable kt;
    auto ktSize = mpiReceive<size_t>(rank);
    while(ktSize--) {
        auto kn = mpiReceive<string>(rank);
        kt._Set(kn, mpiReceive<KeyData*>(rank));
    }
    return kt;
}
