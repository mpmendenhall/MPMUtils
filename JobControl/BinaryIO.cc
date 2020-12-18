/// \file BinaryIO.cc
// -- Michael P. Mendenhall, LLNL 2019

#include "BinaryIO.hh"
#include <cstring> // for std::memcpy

void BinaryWriter::end_wtx() {
    if(!wtxdepth) throw;
    if(--wtxdepth) return;

    if(!wbuff.size()) return;
    _send(wbuff.data(), wbuff.size());
    wbuff.clear();
    flush();
}

void BinaryWriter::append_write(const char* d, size_t n) {
    auto n0 = wbuff.size();
    wbuff.resize(n0 + n);
    std::memcpy(wbuff.data()+n0, d, n);
}

template<>
void BinaryWriter::send<string>(const string& s) {
    start_wtx();
    send<int>(s.size());
    append_write(s.data(), s.size());
    end_wtx();
}

template<>
void BinaryReader::receive<string>(string& s) {
    s = string(receive<int>(), ' ');
    _receive((void*)s.data(), s.size());
}
