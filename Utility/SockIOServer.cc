/// \file SockIOServer.cc

#include "SockIOServer.hh"
#include <unistd.h>    // for write(...), usleep(n)
#include <stdio.h>     // for printf(...)
#include <sys/ioctl.h> // for ioctl(...)
#include <stdexcept>

void SockIOServer::threadjob() {
    create_socket();

    // listen on socket for connections, allowing a backlog of 10
    listen(sockfd, 10);
    printf("Listening for connections on port %i (socket fd %i)\n", port, sockfd);

    // block until new socket created for connection
    while(!all_done) {
        struct sockaddr cli_addr;
        socklen_t clilen = sizeof(cli_addr); // returns actual size of client address
        auto newsockfd = accept(sockfd, &cli_addr, &clilen);
        if (newsockfd < 0) {
            fprintf(stderr, "ERROR %i accepting socket connection!\n", newsockfd);
            continue;
        }
        handle_connection(newsockfd);
    }

    close(sockfd);
}

void SockIOServer::handle_connection(int csockfd) {
    printf("Accepting new connection %i ... and closing it.\n", csockfd);
    close(csockfd);
}

////////////////////
////////////////////
////////////////////

void ConnHandler::handle() {
    printf("Echoing responses from socket fd %i...\n", sockfd);
    int ntries = 0;
    while(ntries++ < 100) {
        int len = 0;
        ioctl(sockfd, FIONREAD, &len);
        if(len > 0) {
            vector<char> buff(len);
            len = read(sockfd, buff.data(), len);
            printf("%i[%i]> '", sockfd, len);
            for(auto c: buff) printf("%c",c);
            printf("'\n");
            ntries = 0;
        } else usleep(100000);
    }
    printf("Closing responder to handle %i.\n", sockfd);
}

////////////////////
////////////////////
////////////////////

void BlockHandler::handle() {
    int32_t bsize;
    while(1) {
        if(abort) break;
        bsize = 0;
        sockread(reinterpret_cast<char*>(&bsize), sizeof(bsize));
        if(bsize > 0) {
             auto buff = alloc_block(bsize);
             if(!buff) break;
             sockread(buff, bsize);
        }
        if(!process(bsize)) break;
    }
}

bool BlockHandler::process(int32_t bsize) {
    if(!bsize || !theblock) return false;
    bool b = process_v(theblock->data);
    return_block();
    return b;
}

bool BlockHandler::process_v(const vector<char>& v) {
    static size_t received = 0;
    static int nprocessed = 0;
    nprocessed++;
    received += v.size();
    if(nprocessed<100 || !(nprocessed % (nprocessed/100))) {
        printf("%i[%zu]> '", sockfd, v.size());
        if(v.size() < 1024) for(auto c: v) printf("%c",c);
        else printf("%.1f MB", received/(1024*1024.));
        printf("'\n");
    }
    return (bool)v.size();
}

char* BlockHandler::alloc_block(int32_t bsize) {
    request_block(bsize);
    if(!theblock) return nullptr;
    theblock->H = this;
    theblock->data.resize(bsize);
    return theblock->data.data();
}

////////////////////
////////////////////
////////////////////

void ConnHandler::threadjob() {
    handle();
    auto ts = dynamic_cast<ThreadedSockIOServer*>(myServer);
    if(ts) ts->handlerClosed(this);
    close(sockfd);
    delete this;
}

void ThreadedSockIOServer::handle_connection(int csockfd) {
    lock_guard<mutex> cl(connsLock);
    auto h = makeHandler(csockfd);
    myConns.insert(h);
    h->launch_mythread();
}

void ThreadedSockIOServer::handlerClosed(ConnHandler* h) {
    lock_guard<mutex> cl(connsLock);
    printf("Removing handler for sockfd %i\n", h->sockfd);
    myConns.erase(h);
}

////////////////////
////////////////////
////////////////////

SockBlockSerializerHandler::SockBlockSerializerHandler(int sfd, SockBlockSerializerServer* SBS):
BlockHandler(sfd, SBS), mySBSS(SBS) { }

void SockBlockSerializerHandler::request_block(int32_t /*bsize*/) {
    theblock = mySBSS->get_allocated();
}

void SockBlockSerializerHandler::return_block() {
    if(theblock) mySBSS->return_allocated(theblock);
    theblock = nullptr;
}
