/// @file SockIOServer.cc

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
    while(runstat != STOP_REQUESTED) {
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
    printf("Accepting new connection %i ...\n", csockfd);
    auto h = makeHandler(csockfd);
    if(!h) throw std::runtime_error("Failed to create socket handler");
    add_thread(h, false);
    h->launch_mythread();
}

void SockIOServer::on_thread_completed(Threadworker* t) {
    printf("Removing handler for sockfd %i\n", t->worker_id);
    close(t->worker_id);
    delete t;
}

ConnHandler* SockIOServer::makeHandler(int sfd) { return new ConnHandler(sfd, this); }

////////////////////
////////////////////
////////////////////

void ConnHandler::threadjob() {
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

void BlockHandler::threadjob() {
    int32_t bsize;
    while(runstat != STOP_REQUESTED) {
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
