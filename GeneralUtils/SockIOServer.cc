/// \file SockIOServer.cc

#ifndef _GNU_SOURCE
#define _GNU_SOURCE // for POLLRDHUP in poll()
#endif

#include "SockIOServer.hh"
#include <netdb.h>     // for sockaddr_in, hostent
#include <string.h>    // for bzero(...)
#include <unistd.h>    // for write(...), usleep(n)
#include <stdio.h>     // for printf(...)
#include <sys/ioctl.h> // for ioctl(...)
#include <poll.h>      // for poll(...)
#include <pthread.h>
#include <cassert>
#include <stdexcept>

bool SockIOServer::process_connections() {
    // open socket file descriptor
    auto sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        fprintf(stderr, "ERROR %i opening socket\n", sockfd);
        throw std::runtime_error("Cannot open socket\n");
        return false;
    }

    // bind server to socket
    struct sockaddr_in serv_addr;
    serv_addr.sin_family = AF_INET;
    if(host.size()) {
        struct hostent* hostinfo = gethostbyname(host.c_str());
        if(hostinfo == nullptr) {
            fprintf (stderr, "ERROR unknown hostname '%s'\n", host.c_str());
            return false;
        }
        serv_addr.sin_addr = *(struct in_addr*) hostinfo->h_addr;
    } else serv_addr.sin_addr.s_addr = htonl(INADDR_ANY); // default for this machine
    serv_addr.sin_port = htons(port); // host to network byte order
    int rc = bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr));
    if(rc < 0) {
        fprintf(stderr, "ERROR %i binding socket\n", rc);
        close(sockfd);
        return false;
    }

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
    auto& A = *(SockIOServer*)p;
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

#ifdef __APPLE__
#define OR_POLLRDHUP
#else
#define OR_POLLRDHUP | POLLRDHUP
#endif

void BlockHandler::handle() {
    int32_t bsize;
    struct pollfd pfd;
    pfd.fd = sockfd;
    pfd.events = POLLIN OR_POLLRDHUP;
    while(1) {
        if(abort) break;
        int ret = poll(&pfd, 1 /*entries in pfd[]*/, block_timeout_ms);
        if(ret != 1 || !(pfd.revents & POLLIN) || (pfd.revents & (POLLERR | POLLHUP | POLLNVAL OR_POLLRDHUP))) break; // timeout or error
        bsize = 0;
        int len = read(sockfd, &bsize, sizeof(bsize));
        if(len != sizeof(bsize)) break; // error condition, e.g connection closed

        if(bsize > 0 && !read_block(bsize)) break;
        if(!process(bsize)) break;
    }
}

bool BlockHandler::read_block(int32_t bsize) {
    //printf("Reading block size %i\n", bsize);
    auto buff = alloc_block(bsize);
    if(!buff) return false;

    int32_t nread = 0;
    struct pollfd pfd;
    pfd.fd = sockfd;
    pfd.events = POLLIN OR_POLLRDHUP;
    while(nread < bsize) {
        int ret = poll(&pfd, 1 /*entries in pfd[]*/, read_timeout_ms);
        if(ret != 1 || !(pfd.revents & POLLIN) || (pfd.revents & (POLLERR | POLLHUP | POLLNVAL OR_POLLRDHUP))) break; // timeout or error

        auto len = read(sockfd, buff+nread, bsize-nread);
        if(len < 0) return false;
        nread += len;
        if(nread != bsize) usleep(1000);
    }
    return true;
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
    return v.size();
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
    auto h = (ConnHandler*)p;
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
BlockHandler(sfd, SBS), myServer(SBS) { }

void SockBlockSerializerHandler::request_block(int32_t /*bsize*/) {
    theblock = myServer->get_allocated();
}

void SockBlockSerializerHandler::return_block() {
    if(theblock) myServer->return_allocated(theblock);
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
