/// \file BinaryIO.cc
// Michael P. Mendenhall, LLNL 2019

#include "BinaryIO.hh"

void BinaryIO::end_wtx() {
    assert(wtxdepth);
    if(--wtxdepth) return;
    if(!buff.size()) return;
    _send(buff.data(), buff.size());
    buff.clear();
}

void BinaryIO::append_write(const char* d, size_t n) {
    auto n0 = buff.size();
    buff.resize(n0 + n);
    memcpy(buff.data()+n0, d, n);
}

template<>
void BinaryIO::send<string>(const string& s) {
    start_wtx();
    send<int>(s.size());
    append_write(s.data(), s.size());
    end_wtx();
}

template<>
void BinaryIO::receive<string>(string& s) {
    s = string(receive<int>(), ' ');
    _receive((void*)s.data(), s.size());
}
