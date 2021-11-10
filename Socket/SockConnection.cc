/// \file SockConnection.cc

#include "SockConnection.hh"
#include "to_str.hh"

#ifndef _GNU_SOURCE
// for POLLRDHUP in poll()
#define _GNU_SOURCE
#endif

#include <string.h> // for bzero(...)
#include <stdio.h>  // for printf(...)
#include <errno.h>  // for errno
#include <poll.h>   // for poll(...)
#include <signal.h> // for SIGPIPE

#ifdef __APPLE__
#define OR_POLLRDHUP
#else
#define OR_POLLRDHUP | POLLRDHUP
#endif

void SockFD::open_sockfd() {
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if(sockfd < 0) {
        sockfd = 0;
        throw std::runtime_error("Cannot open any socket");
    }
}

void SockFD::sockwrite(char* buff, size_t nbytes) {
    int nretries = 3;
    while(nbytes) {
        auto ret = write(sockfd, buff, nbytes);
        if(ret <= 0) {
            if(nretries--) {
                usleep(1000);
                continue;
            }
            throw std::runtime_error("Failed writing " + to_str(nbytes) + " to socket, error " + to_str(ret));
        }
        nbytes -= ret;
        buff += ret;
    }
}

void SockFD::sockread(char* buff, size_t nbytes) {
    size_t nread = 0;
    struct pollfd pfd;
    pfd.fd = sockfd;
    pfd.events = POLLIN OR_POLLRDHUP;

    while(nread < nbytes) {
        int ret = poll(&pfd, 1 /*entries in pfd[]*/, read_timeout_ms);
        if(ret < 0) throw std::runtime_error("poll() failure, error " + to_str(errno));
        if(!ret) throw std::runtime_error("socket read timeout");
        if(ret != 1) throw std::runtime_error("poll() returned " + to_str(ret));
        if(!(pfd.revents & POLLIN) || (pfd.revents & (POLLERR | POLLHUP | POLLNVAL OR_POLLRDHUP)))
            throw std::runtime_error("socket poll() error or hangup");

        auto len = read(sockfd, buff+nread, nbytes-nread);
        if(len < 0) throw std::runtime_error("Failed socket read, error " + to_str(len));
        nread += len;
        if(nread != nbytes) usleep(1000);
    }
}

int SockFD::awaitConnection() {
    listen(sockfd, 1);
    struct sockaddr cli_addr;
    socklen_t clilen = sizeof(cli_addr); // returns actual size of client address
    auto newsockfd = accept(sockfd, &cli_addr, &clilen);
    if(newsockfd < 0) throw std::runtime_error("failed to accept connection");
    return newsockfd;
}

//----------------------------------

void SockConnection::configure_host() {
    bzero((char*) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(port); // host to network byte order

    if(host.size()) {
        server = gethostbyname(host.c_str());
        if(!server) throw std::runtime_error("Unknown hostname '" + host + "'");
        serv_addr.sin_addr = *(struct in_addr*) server->h_addr;
        bcopy(server->h_addr, (char*)&serv_addr.sin_addr.s_addr, server->h_length);
    } else serv_addr.sin_addr.s_addr = htonl(INADDR_ANY); // default for this machine
}

void SockConnection::create_socket() {
    open_sockfd();
    configure_host();
    int rc = bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr));
    if(rc < 0) {
        close_socket();
        throw std::runtime_error("Cannot bind to socket " + host + ":" + to_str(port) + " (error "+to_str(rc)+")");
    }
}

void SockConnection::connect_to_socket() {
    open_sockfd();
    configure_host();
    int rc = connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr));
    if(rc < 0) {
        close_socket();
        throw std::runtime_error("Cannot connect to socket  " + host + ":" + to_str(port) + " (error "+to_str(rc)+")");
    }
    signal(SIGPIPE, SIG_IGN); // ignore SIGPIPE errors; handle by return in code
}
