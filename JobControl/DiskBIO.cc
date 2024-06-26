/// @file DiskBIO.cc

#include "DiskBIO.hh"
#include <sys/types.h>
#include <fcntl.h>

void FDBinaryWriter::_send(const void* vptr, size_t size) {
    if(!vptr || fOut < 0) throw std::logic_error("invalid object write");
    if(size != (size_t)write(fOut, vptr, size)) throw std::runtime_error("Can't write file");
}

void FDBinaryReader::openIn(const string& s) {
    if(fIn >= 0) close(fIn);
    fIn = s.size()? open(s.c_str(), O_RDONLY) : -1;
}

void FDBinaryWriter::openOut(const string& s, bool append) {
    if(fOut >= 0) close(fOut);
    fOut = -1;
    if(s.size()) {
        auto flags = O_WRONLY | O_CREAT;
        if(append) flags = flags | O_APPEND;
        fOut = open(s.c_str(), flags, S_IRUSR | S_IWUSR);
    }
    if(s.size() && fOut < 0) throw std::runtime_error("Failure opening output file!");
}

void FDBinaryWriter::closeOut() {
    if(fOut >= 0 && close(fOut)) throw std::runtime_error("Failure closing output file!");
    fOut = -1;
}
