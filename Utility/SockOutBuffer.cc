/// \file SockOutBuffer.cc

#include "SockOutBuffer.hh"
#include <stdio.h>  // for printf(...)
#include <errno.h>  // for errno

void SockOutBuffer::process_item() {
    if(sockfd) {
        int32_t bsize = current.size();
        auto ret = sockwrite((char*)&bsize, sizeof(bsize));
        if(ret > 0 && bsize) ret = sockwrite(current.data(), bsize);
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
    SockOutBuffer SIB;
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
