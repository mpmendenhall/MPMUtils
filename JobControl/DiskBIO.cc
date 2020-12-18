/// \file DiskBIO.cc

#include "DiskBIO.hh"
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

void FDBinaryWriter::_send(void* vptr, int size) {
    if(!vptr || fOut < 0 || size == -1) throw;
    if(fOut >= 0 && size != write(fOut, vptr, size))
        throw std::runtime_error("Can't write file!");
}

void FDBinaryReader::openIn(const string& s) {
    if(fIn >= 0) close(fIn);
    fIn = s.size()? open(s.c_str(), O_RDONLY) : -1;
}

void FDBinaryWriter::openOut(const string& s) {
    if(fOut >= 0) close(fOut);
    fOut = s.size()? open(s.c_str(), O_WRONLY | O_CREAT | O_APPEND, S_IRUSR | S_IWUSR) : -1;
    if(s.size() && fOut < 0) throw std::runtime_error("Failure opening output file!");
}

void FDBinaryWriter::closeOut() {
    if(fOut >= 0 && close(fOut)) throw std::runtime_error("Failure closing output file!");
    fOut = -1;
}
