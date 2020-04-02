/// \file SockConnection.cc

#include "SockConnection.hh"
#include <string.h> // for bzero(...)
#include <stdio.h>  // for printf(...)
#include <signal.h> // for SIGPIPE
#include <errno.h>  // for errno

bool SockConnection::open_socket(const string& host, int port) {
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        fprintf(stderr, "ERROR opening socket\n");
        return false;
    }

    server = gethostbyname(host.c_str());
    if(server == nullptr) {
        fprintf(stderr, "ERROR: Host '%s' not found!\n",host.c_str());
        close_socket();
        return false;
    }

    bzero((char*) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    bcopy((char*)server->h_addr, (char*)&serv_addr.sin_addr.s_addr, server->h_length);
    serv_addr.sin_port = htons(port);
    if(connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        fprintf(stderr, "ERROR connecting to socket %s:%i\n", host.c_str(), port);
        close_socket();
        return false;
    }

    signal(SIGPIPE, SIG_IGN); // ignore SIGPIPE errors; handle by return in code

    return true;
}

int SockConnection::sockwrite(char* buff, size_t nbytes) {
    int nwritten = 0;
    int nretries = 3;
    while(nbytes) {
        auto ret = write(sockfd, buff, nbytes);
        if(ret <= 0) {
            if(nretries--) {
                usleep(1000);
                continue;
            }
            perror("Error writing to socket");
            return ret;
        }
        nwritten += ret;
        nbytes -= ret;
        buff += ret;
    }
    return nwritten;
}
