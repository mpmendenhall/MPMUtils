/// \file DiskIOJobControl.cc
// -- Michael P. Mendenhall, LLNL 2019

#include "DiskIOJobControl.hh"
#include <sstream>

/*
void DiskIOJobControl::init(int argc, char **argv) {
    rank = 0;
    for(int i=1; i<argc-1; i++) if(argv[i][0]=='-' && argv[i][1]=='N') rank = atoi(argv[i+1]);
    if(verbose > 2) printf("Initializing DiskIOJobControl[%i] for '%s'\n", rank, argv[0]);
}

DiskIOJobControl::~DiskIOJobControl() {
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
    FDBinaryWriter b(fn.str());
    b.send(vptr, size);
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
*/
