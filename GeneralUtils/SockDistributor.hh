/// \file SockDistributor.hh One-to-many sockets binary data push distribution
// Michael P. Mendenhall, 2018

#ifndef SOCKDISTRIBUTOR_HH
#define SOCKDISTRIBUTOR_HH

#include "SockIOServer.hh"
#include "SockOutBuffer.hh"

/// Output distribution handler; uses SockOutBuffer to send data
class SockDistribHandler: public ConnHandler, public SockOutBuffer {
public:
    /// Constructor
    SockDistribHandler(int sfd, SockIOServer* s = nullptr): ConnHandler(sfd,s) { SockOutBuffer::sockfd = sfd; SockOutBuffer::launch_mythread(); }

    /// handle responses from client (do nothing; wait for output to break)
    void handle() override {
        while(SockOutBuffer::sockfd) usleep(10000);
        SockOutBuffer::finish_mythread();
    }
};

/// Server for distributing block data to listening clients
class SockDistribServer: public ThreadedSockIOServer {
public:
    /// Constructor
    SockDistribServer() { }

    /// send data to connected clients
    void sendData(const char* d, size_t n);
    /// send vector as binary blob
    template<typename T>
    void sendvector(const vector<T>& v) { sendData((char*)v.data(), v.size()*sizeof(T)); }

    // set: host, port
    // call: void process_connections_thread();
    // call: sendData(...)
    // set: accept_connections = false;

protected:
    /// create correct handler type
    ConnHandler* makeHandler(int sfd) override { return new SockDistribHandler(sfd,this); }
};

/// Client requesting and receiving block data from server
class SockDistribClient: public SockConnection, public BlockHandler {
public:
    /// Constructor
    SockDistribClient(const string& host = "", int port = 0):
    BlockHandler(0,nullptr) { if(host.size() && port) open_socket(host,port); }

    /// Open connection and receive block response
    bool open_socket(const string& host, int port) override;

    // subclass me: Process data after buffer read; return false to end communication
    // bool process(const vector<char>& v) overide;
};


/// client with re-casting function call
template<typename T>
class SockDistribClientT: public SockDistribClient {
public:
    using SockDistribClient::SockDistribClient;
protected:
    /// Process received data as array of T
    bool process(const vector<char>& v) override { return process((T*)v.data(), v.size()/sizeof(T)); }
    /// Process received data as array of T --- Subclass me!
    virtual bool process(const T*, size_t n) { return n; }
};

#endif
