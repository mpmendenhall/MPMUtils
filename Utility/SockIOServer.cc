/// \file SockIOServer.cc

#include "SockIOServer.hh"
#include <unistd.h>    // for write(...), usleep(n)
#include <stdio.h>     // for printf(...)
#include <sys/ioctl.h> // for ioctl(...)
#include <stdexcept>

bool SockIOServer::process_connections() {
    create_socket();

    // listen on socket for connections, allowing a backlog of 10
    listen(sockfd, 10);
    printf("Listening for connections on port %i (socket fd %i)\n", port, sockfd);

    // block until new socket created for connection
    accept_connections = true;
    while(accept_connections) {
        { std::lock_guard<std::mutex> al(acceptLock); }
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
    return true;
}

void SockIOServer::handle_connection(int csockfd) {
    printf("Accepting new connection %i ... and closing it.\n", csockfd);
    close(csockfd);
}

void* sockioserver_thread(void* p) {
    auto& A = *reinterpret_cast<SockIOServer*>(p);
    auto isListening = A.process_connections();
    if(!isListening) exit(EXIT_FAILURE);
    return nullptr;
}

void SockIOServer::process_connections_thread() {
    pthread_create(&sockthread, nullptr, sockioserver_thread, this);
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

void* run_sockio_thread(void* p) {
    auto h = reinterpret_cast<ConnHandler*>(p);
    h->handle();
    auto ts = dynamic_cast<ThreadedSockIOServer*>(h->myServer);
    if(ts) ts->handlerClosed(h);
    close(h->sockfd);
    delete h;
    return nullptr;
}

void ThreadedSockIOServer::handle_connection(int csockfd) {
    std::lock_guard<std::mutex> cl(connsLock);
    pthread_t mythread;
    auto h = makeHandler(csockfd);
    myConns.insert(h);
    pthread_create(&mythread, nullptr, // thread attributes
                   run_sockio_thread, h);
}

void ThreadedSockIOServer::handlerClosed(ConnHandler* h) {
    std::lock_guard<std::mutex> cl(connsLock);
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


#ifdef SOCKET_TEST
// make SockIOServer -j4; ./SockIOServer
class TestIOServer: public ThreadedSockIOServer {
protected:
    ConnHandler* makeHandler(int sfd) override { return new BlockHandler(sfd); }
};

int main(int, char **) {
    SockBlockSerializerServer SBS;
    SBS.launch_mythread();
    SBS.process_connections("localhost",9999);
}
#endif
