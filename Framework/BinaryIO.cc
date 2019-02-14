/// \file BinaryIO.cc
// Michael P. Mendenhall, LLNL 2019

#include "BinaryIO.hh"

template<>
void BinaryIO::send<string>(const string& s) {
    send<int>(s.size());
    _send((void*)s.data(), s.size());
}

template<>
void BinaryIO::receive<string>(string& s) {
    int length = receive<int>();
    s = string(length, ' ');
    _receive(&s[0], length);
}
