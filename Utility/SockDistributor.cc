/// \file SockDistributor.cc
// -- Michael P. Mendenhall, 2018

#include "SockDistributor.hh"
#include <cassert>

void SockDistribServer::sendData(const char* d, size_t n) {
    std::lock_guard<std::mutex> cl(connsLock);
    //printf("Sending %zu bytes data to %zu connections\n", n, myConns.size());
    for(auto c: myConns) {
        auto cc = static_cast<SockDistribHandler*>(c);
        auto v = cc->get_writepoint();
        if(!v) continue;
        v->assign(d,d+n);
        cc->finish_write();
    }
}
