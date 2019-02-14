/// \file DiskIOJobControl.cc
// Michael P. Mendenhall, LLNL 2019

#include "DiskIOJobControl.hh"
#include <iostream>
#include <fstream>
#include <sstream>

void DiskIOJobControl::init(int argc, char **argv) {
    rank = 0;
    for(int i=1; i<argc-1; i++) if(argv[i][0]=='-' && argv[i][1]=='N') rank = atoi(argv[i+1]);
    if(verbose > 2) printf("Initializing DiskIOJobControl[%i] for '%s'\n", rank, argv[0]);

    persistent = !rank;
    runLocal = false;

    if(rank) {

        std::stringstream fn;
        fn << data_bpath << "/SavedState_" << rank << ".dat";
        std::ifstream i(fn.str());
        if(i.good()) {
            if(verbose > 2) printf("Loading saved state from '%s'\n", fn.str().c_str());
            IOStreamBIO b(&i, nullptr);
            size_t s = b.receive<size_t>();
            while(s--) {
                auto k = b.receive<string>();
                b.receive(stateData[k].dead);
            }
        } else if(verbose > 2) printf("No saved state available at '%s'\n", fn.str().c_str());

    } else {
        int ret = std::system(("rm -f "+data_bpath+"/SavedState_*.dat").c_str());
        ret = std::system(("rm -f "+data_bpath+"/CommBuffer_*.dat").c_str());
        if(ret) exit(111);
    }
}

void DiskIOJobControl::finish() {
    std::stringstream fn;
    fn << data_bpath << "/SavedState_" << rank << ".dat";
    std::ofstream o(fn.str());
    IOStreamBIO b(nullptr, &o);

    b.send<size_t>(stateData.size());
    for(auto& kv: stateData) {
        b.send(kv.first);
        b.send(kv.second.dead);
    }
    o.close();

    if(rank) {
        std::stringstream fn2;
        fn2 << "rm -f " << data_bpath << "/CommBuffer_0_to_" << rank << ".dat";
        int ret = std::system(fn2.str().c_str());
        if(ret) exit(112);
    }
}

void DiskIOJobControl::_send(void* vptr, int size) {
    std::ofstream of;
    std::stringstream fn;
    fn << data_bpath << "/CommBuffer_" << rank << "_to_" << dataDest << ".dat";
    of.open(fn.str(),  std::ios_base::app);
    of.write((char*)vptr, size);
    of.close();
}

void DiskIOJobControl::_receive(void* vptr, int size) {
    auto& p = srcpos[dataSrc];

    std::stringstream fn;
    fn << data_bpath << "/CommBuffer_" << dataSrc << "_to_" << rank << ".dat";
    std::ifstream fin;

    do { // wait for file available
        fin.open(fn.str(), std::ios_base::binary);
        if(fin.good() && fin.is_open()) break;
        usleep(100000);
    } while(true);

    do { // wait for data available
        fin.seekg(0, std::ios_base::end);
        auto fsize = fin.tellg();
        if(fsize >= int(p+size)) break;
        usleep(100000);
    } while(true);

    fin.seekg(p);
    fin.read((char*)vptr, size);
    p += size;
    fin.close();
}
