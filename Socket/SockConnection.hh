/// \file SockConnection.hh Base class for connecting to a socket

#ifndef SOCKCONNECTION_HH
#define SOCKCONNECTION_HH

#include <netdb.h>  // for sockaddr_in, hostent
#include <unistd.h> // for write(...), close(...), usleep(n)
#include <stdexcept>
#include <string>
using std::string;

/// read/write from a socket file descriptor
class SockFD {
public:
    /// Constructor
    explicit SockFD(int s = 0): sockfd(s) { }
    /// Destructor
    virtual ~SockFD() { close_socket(); }

    /// close socket
    void close_socket() { if(sockfd) close(sockfd); sockfd = 0; }

    /// write to socket; throw on failure
    void sockwrite(char* buff, size_t nbytes);
    /// blocking read from socket
    void sockread(char* buff, size_t nbytes);

    int sockfd = 0; ///< file descriptor number for socket
    int read_timeout_ms = -1; ///< read timeout [ms]; negative for infinite

    /// blocking wait for one new connection; return connection file descriptor
    int awaitConnection();

protected:
    /// open socket file descriptor
    void open_sockfd();
};

/// Socket connection wrapper
class SockConnection: public SockFD {
public:
    /// Constructor
    explicit SockConnection(const string& _host = "", int _port = 0): host(_host), port(_port) { }
    /// Constrcutor with (already open) file descriptor
    explicit SockConnection(int sfd): SockFD(sfd) { }

    /// connect to open socket; throw on failure
    virtual void connect_to_socket();
    /// (try to) make socket connection, specifying host/port
    void connect_to_socket(const string& _host, int _port) { host = _host; port = _port; connect_to_socket(); }
    /// bind to socket to accept connections; throw on failure
    virtual void create_socket();

    string host;    ///< hostname
    int port = 0;   ///< socket port

protected:
    /// get host info for server
    void configure_host();

    struct sockaddr_in serv_addr;       ///< server address data
    struct hostent* server = nullptr;   ///< server
};

#endif
