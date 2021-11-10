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
