/// \file DiskIOJobControl.cc
// Michael P. Mendenhall, LLNL 2019

#include "DiskIOJobControl.hh"
#include <iostream>
#include <fstream>
#include <sstream>
#include <sys/types.h>
#include <fcntl.h>
#include <cstdlib> // for system(...)

int runSysCmd(const string& cmd) {
    int ret = std::system(cmd.c_str());
    if(ret) {
        auto e = WEXITSTATUS(ret);
        printf("*** '%s' exited with return %i!\n", cmd.c_str(), e);
        exit(e);
    }
    return ret;
}

void FDBinaryIO::openIn(const string& s) {
    if(fIn >= 0) close(fIn);
    fIn = s.size()? open(s.c_str(), O_RDONLY) : -1;
}

void FDBinaryIO::openOut(const string& s) {
    if(fOut >= 0) close(fOut);
    fOut = s.size()? open(s.c_str(), O_WRONLY | O_CREAT | O_APPEND, S_IRUSR | S_IWUSR) : -1;
    if(s.size() && fOut < 0) exit(99);
}

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

    } else runSysCmd("rm -f "+data_bpath+"/SavedState_*.dat");
}

void DiskIOJobControl::finish() {
    std::stringstream fn;
    fn << data_bpath << "/SavedState_" << rank << ".dat";

    if(stateData.size()) {
        FDBinaryIO b("",fn.str());
        b.send<size_t>(stateData.size());
        for(auto& kv: stateData) {
            b.send(kv.first);
            b.send(kv.second.dead);
        }
    } else runSysCmd("rm -f "+fn.str());

    if(!rank) runSysCmd("rm -f " + data_bpath + "/CommBuffer_0_to_*.dat");
}

void DiskIOJobControl::clearOut() {
    std::stringstream fn;
    fn << "rm -f " << data_bpath << "/CommBuffer_" << rank << "_to_" << dataDest << ".dat";
    if(verbose > 4) printf("clearOut %s\n", fn.str().c_str());
    runSysCmd(fn.str());
}

void DiskIOJobControl::clearIn() {
    srcpos[dataSrc] = 0;
    std::stringstream fn;
    fn << "rm -f " << data_bpath << "/CommBuffer_" << dataSrc << "_to_" << rank << ".dat";
    if(verbose > 4) printf("clearIn %s\n", fn.str().c_str());
    runSysCmd(fn.str());
}

void DiskIOJobControl::_send(void* vptr, int size) {
    std::stringstream fn;
    fn << data_bpath << "/CommBuffer_" << rank << "_to_" << dataDest << ".dat";
    FDBinaryIO b("", fn.str());
    b._send((char*)vptr, size);
}

void DiskIOJobControl::_receive(void* vptr, int size) {
    auto& p = srcpos[dataSrc];

    std::stringstream fn;
    fn << data_bpath << "/CommBuffer_" << dataSrc << "_to_" << rank << ".dat";
    std::ifstream fin;

    do { // wait for file available
        fin.open(fn.str(), std::ios_base::binary);
        if(fin.good() && fin.is_open()) break;
        fin.close();
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
