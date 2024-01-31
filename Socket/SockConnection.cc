/// @file

#include "SockConnection.hh"

#ifndef _GNU_SOURCE
// for POLLRDHUP in poll()
#define _GNU_SOURCE
#endif

#include <string.h> // for bzero(...), strerror
#include <stdio.h>  // for printf(...)
#include <errno.h>  // for errno
#include <poll.h>   // for poll(...)
#include <signal.h> // for SIGPIPE
#include <sys/ioctl.h>

#ifdef __APPLE__
#define OR_POLLRDHUP
#else
#define OR_POLLRDHUP | POLLRDHUP
#endif

void SockFD::open_sockfd() {
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if(sockfd < 0) {
        sockfd = 0;
        throw SockFDerror(*this, "Cannot open any socket");
    }
}

void SockFD::sockwrite(const char* buff, size_t nbytes, bool fail_ok) {
    int nretries = 3;
    while(nbytes) {
        auto ret = write(sockfd, buff, nbytes);
        if(ret <= 0) {
            if(nretries--) {
                usleep(1000);
                continue;
            }
            if(fail_ok) return;
            string emsg = "Failed writing " + to_str(nbytes) + " to socket; return " + to_str(ret);
            emsg += +" with error " + to_str(errno) + " " + strerror(errno);
            throw SockFDerror(*this,  emsg);
        }
        nbytes -= ret;
        buff += ret;
    }
}

bool SockFD::do_poll(bool fail_ok) {
    struct pollfd pfd;
    pfd.fd = sockfd;
    pfd.events = POLLIN OR_POLLRDHUP;

    do {
        int ret = poll(&pfd, 1 /*entries in pfd[]*/, read_timeout_ms);
        if(ret != 1) {
            if(fail_ok) return false;
            if(ret < 0) {
                if(errno == EINTR) continue; // ignore interrupts TODO what are we getting?
                throw SockFDerror(*this, "poll() failure, error " + to_str(errno) + " " + strerror(errno));
            }
            if(!ret) throw SockFDerror(*this, "socket read timeout");
            throw SockFDerror(*this, "poll() unexpectedly returned " + to_str(ret));
        }
    } while(false);

    if(!(pfd.revents & POLLIN) || (pfd.revents & (POLLERR | POLLHUP | POLLNVAL OR_POLLRDHUP))) {
        if(fail_ok) return false;
        if(!(pfd.revents & POLLIN)) throw SockFDerror(*this, "poll() results lack POLLIN");
        if(pfd.revents & POLLERR) throw SockFDerror(*this, "poll() returned POLERR");
        if(pfd.revents & POLLHUP) throw SockFDerror(*this, "poll() returned POLHUP");
        if(pfd.revents & POLLNVAL) throw SockFDerror(*this, "poll() returned POLLNVAL");
        throw SockFDerror(*this, "poll() returned POLLRDHUP");
    }

    return true;
}

size_t SockFD::sockread(char* buff, size_t nbytes, bool fail_ok) {
    size_t nread = 0;

    while(nread < nbytes) {
        auto len = read(sockfd, buff+nread, nbytes-nread);
        if(len < 0) {
            if(fail_ok) return nread;
            throw SockFDerror(*this, "Failed socket read, error " + to_str(errno) + " " + strerror(errno));
        }
        nread += len;
        if(nread == nbytes) return nread;
        //if(!len) usleep(1000);
        if(!do_poll(fail_ok)) return nread;
    }

    return nread;
}

size_t SockFD::sockread_upto(char* buff, size_t nbytes) {
    auto len = read(sockfd, buff, nbytes);
    if(len < 0) return 0;
    return len;
}

void SockFD::vecread(vector<char>& v, bool fail_ok) {
    v.clear();
    if(!do_poll(fail_ok)) return;
    int count;
    ioctl(sockfd, FIONREAD, &count);
    v.resize(count);
    auto len = read(sockfd, v.data(), count);
    if(len < 0) {
        if(fail_ok) return;
        throw SockFDerror(*this, "Failed socket read, error " + to_str(len));
    }
    v.resize(len);
}

int SockFD::awaitConnection() {
    listen(sockfd, 1);
    struct sockaddr cli_addr;
    socklen_t clilen = sizeof(cli_addr); // returns actual size of client address
    auto newsockfd = accept(sockfd, &cli_addr, &clilen);
    if(newsockfd < 0) throw SockFDerror(*this, "failed to accept connection");
    return newsockfd;
}

//----------------------------------

void SockConnection::configure_host() {
    bzero(reinterpret_cast<char*>(&serv_addr), sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(port); // host to network byte order

    if(host.size()) {
        server = gethostbyname(host.c_str());
        if(!server) throw sockerror(*this, "Unknown hostname '" + host + "'");
        serv_addr.sin_addr = *reinterpret_cast<struct in_addr*>(server->h_addr);
        bcopy(server->h_addr, reinterpret_cast<char*>(&serv_addr.sin_addr.s_addr), server->h_length);
    } else serv_addr.sin_addr.s_addr = htonl(INADDR_ANY); // default for this machine
}

void SockConnection::create_socket() {
    open_sockfd();
    configure_host();
    int rc = bind(sockfd, reinterpret_cast<struct sockaddr*>(&serv_addr), sizeof(serv_addr));
    if(rc < 0) {
        close_socket();
        throw sockerror(*this, "Cannot bind to socket (error "+to_str(rc)+")");
    }
}

void SockConnection::connect_to_socket() {
    open_sockfd();
    configure_host();
    int rc = connect(sockfd, reinterpret_cast<struct sockaddr*>(&serv_addr), sizeof(serv_addr));
    if(rc < 0) {
        close_socket();
        throw sockerror(*this, "Cannot connect to socket (error "+to_str(rc)+")");
    }
    signal(SIGPIPE, SIG_IGN); // ignore SIGPIPE errors; handle by return in code
}
