/// \file SockIOBuffer.hh Buffered socket data transfer

#ifndef SOCKIOBUFFER_HH
#define SOCKIOBUFFER_HH

#include "LocklessCircleBuffer.hh"

#include <netdb.h> // for sockaddr_in, hostent
#include <string>
using std::string;
#include <vector>
using std::vector;

/// Buffered data block output to socket connection
class SockIOBuffer: public LocklessCircleBuffer<vector<char>> {
public:
    /// Constructor
    SockIOBuffer(const string& host = "", int port = 0, size_t nbuff = 1000):
    LocklessCircleBuffer(nbuff) { sleep_us = 1000; if(host.size() && port) open_socket(host,port); }
    /// Destructor
    virtual ~SockIOBuffer() { close_socket(); }

    /// (try to) open socket connection
    bool open_socket(const string& host, int port);
    /// close socket
    void close_socket() { close(sockfd); sockfd = 0; }

    // use SockIOData* LocklessCircleBuffer::get_writepoint() and finish_write()
    // to push new data onto sending queue

protected:
    /// send data block
    void process_item() override;

    int sockfd = 0;                         ///< file descriptor number for socket
    struct sockaddr_in serv_addr;           ///< server address data
    struct hostent* server = nullptr;       ///< server
};

#endif
