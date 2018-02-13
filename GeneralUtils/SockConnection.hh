/// \file SockConnection.hh Base class for connecting to a socket

#ifndef SOCKCONNECTION_HH
#define SOCKCONNECTION_HH

#include <netdb.h>  // for sockaddr_in, hostent
#include <unistd.h> // for write(...), close(...)
#include <signal.h>
#include <string>
using std::string;

/// Socket connection wrapper
class SockConnection {
public:
    /// Constructor
    SockConnection(const string& host = "", int port = 0) { if(host.size() && port) open_socket(host,port); }
    /// Destructor
    virtual ~SockConnection() { close_socket(); }

    /// (try to) open socket connection
    virtual bool open_socket(const string& host, int port);
    /// close socket
    void close_socket() { if(sockfd) close(sockfd); sockfd = 0; }
    /// try writing to socket
    int sockwrite(char* buff, size_t nbytes);

protected:

    int sockfd = 0;                     ///< file descriptor number for socket
    struct sockaddr_in serv_addr;       ///< server address data
    struct hostent* server = nullptr;   ///< server
};

#endif
