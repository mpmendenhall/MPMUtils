/// \file SockDistributor.cc
// -- Michael P. Mendenhall, 2018

#include "SockDistributor.hh"
#include <cassert>

void SockDistribServer::sendData(const char* d, size_t n) {
    lock_guard<mutex> cl(inputMut);
    //printf("Sending %zu bytes data to %zu connections\n", n, myConns.size());

    for(auto c: mythreads) {
        auto cc = dynamic_cast<SockDistribHandler*>(c);
        if(!cc) throw std::logic_error("incorrect handler type");
        auto v = cc->SOB.get_writepoint();
        if(!v) continue;
        v->assign(d,d+n);
        cc->SOB.finish_write();
    }
}
