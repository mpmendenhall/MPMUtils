/// \file SockIOBuffer.cc

#include "SockIOBuffer.hh"
#include <string.h> // for bzero(...)
#include <unistd.h> // for write(...)
#include <stdio.h>  // for printf(...)
#include <signal.h> // for SIGPIPE
#include <errno.h>  // for errno

bool SockIOBuffer::open_socket(const string& host, int port) {
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
    bcopy(server->h_addr, (char*)&serv_addr.sin_addr.s_addr, server->h_length);
    serv_addr.sin_port = htons(port);
    if(connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        fprintf(stderr, "ERROR connecting to socket %s:%i\n", host.c_str(), port);
        close_socket();
        return false;
    }

    signal(SIGPIPE, SIG_IGN); // ignore SIGPIPE errors; handle by return in code

    return true;
}

int sockwrite(int fd, char* buff, size_t nbytes) {
    int nwritten = 0;
    int nretries = 3;
    while(nbytes) {
        auto ret = write(fd, buff, nbytes);
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

void SockIOBuffer::process_item() {
    if(sockfd) {
        int32_t bsize = current.size();
        auto ret = sockwrite(sockfd, (char*)&bsize, sizeof(bsize));
        if(ret > 0 && bsize) ret = sockwrite(sockfd, current.data(), bsize);
        if(ret <= 0) {
            fprintf(stderr, "ERROR %i writing to socket descriptor %i; connection closed!\n", errno, sockfd);
            close_socket();
        }
    }
    current.clear();
}

////////////////////
////////////////////
////////////////////

#ifdef SOCKET_BUFFTEST
int main(int, char **) {
    SockIOBuffer SIB;
    SIB.launch_mythread();
    SIB.open_socket("localhost",9999);

    for(int i=0; i<10; i++) {
        printf("Sending some data...\n");
        auto wp = SIB.get_writepoint();
        for(int j=0; j<10; j++) wp->push_back('*');
        SIB.finish_write();
        usleep(1000000);
    }
}
#endif
