/// \file SockOutBuffer.cc

#include "SockOutBuffer.hh"
#include <stdio.h>  // for printf(...)
#include <errno.h>  // for errno

void SockOutBuffer::process_item() {
    if(sockfd) {
        int32_t bsize = current.size();
        try {
            sockwrite(current.data(), bsize);
        } catch(std::runtime_error& e) {
            fprintf(stderr, "%s\n\tclosing socket descriptor %i\n", e.what(), sockfd);
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
    SockOutBuffer SIB("localhost", 9999);
    SIB.launch_mythread();
    SIB.connect_to_socket();

    for(int i=0; i<10; i++) {
        printf("Sending some data...\n");
        auto wp = SIB.get_writepoint();
        for(int j=0; j<10; j++) wp->push_back('*');
        SIB.finish_write();
        usleep(1000000);
    }
}
#endif
