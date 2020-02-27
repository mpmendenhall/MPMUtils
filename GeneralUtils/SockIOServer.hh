/// \file SockIOServer.hh I/O server for multiple socket connections

#ifndef SOCKIOSERVER_HH
#define SOCKIOSERVER_HH

#include <string>
using std::string;
#include <vector>
using std::vector;
#include <set>
using std::set;
#include <mutex>

class ConnHandler;

/// Base class listening and handling connections to port
class SockIOServer {
public:
    /// Constructor
    SockIOServer() { }
    /// Destructor
    virtual ~SockIOServer() { }

    string host = "localhost";          ///< host to serve on
    int port = 0;                       ///< port to serve on
    bool accept_connections = false;    ///< whether to accept new connections

    /// receive and process connections to host and port
    bool process_connections();
    /// launch process_connections in a separate thread
    void process_connections_thread();

protected:
    std::mutex acceptLock;              ///< lock on accepting new connections

    /// handle each new connection --- subclass me!
    virtual void handle_connection(int csockfd);
    /// handler-creating thread
    pthread_t sockthread;
};

/// Base class for handling one accepted connection
class ConnHandler {
public:
    /// Constructor
    explicit ConnHandler(int sfd, SockIOServer* s = nullptr): sockfd(sfd), myServer(s) { }
    /// Destructor
    virtual ~ConnHandler() { }
    /// Communicate with accepted connection
    virtual void handle();

    int sockfd;             ///< accepted connection file descriptor
    SockIOServer* myServer; ///< spawning parent server
};

/// Socket server spawning threads for each connection
class ThreadedSockIOServer: public SockIOServer {
public:

    /// callback on connection handler close (needs to be thread-safe; runs in handler's thread)
    virtual void handlerClosed(ConnHandler* h);

protected:
    /// create correct handler type (runs in handle_connection() thread)
    virtual ConnHandler* makeHandler(int sfd) { return new ConnHandler(sfd, this); }
    /// handle each new connection, by spawning new thread with makeHandler() object
    void handle_connection(int csockfd) override;

    set<ConnHandler*> myConns;  ///< list of active connections
    std::mutex connsLock;       ///< lock on myConns
};

//////////////////////////////
//////////////////////////////

/// Simple block data transfer protocol: int32_t bsize, data[bsize]
class BlockHandler: public ConnHandler {
public:
    /// Constructor
    explicit BlockHandler(int sfd, SockIOServer* s = nullptr): ConnHandler(sfd, s) { }
    /// Destructor
    ~BlockHandler() { delete theblock; }
    /// Receive block size and whole of expected data
    void handle() override;

    /// received data block with recipient identifier
    struct dblock {
        BlockHandler* H;    ///< pointer back to this handler
        vector<char> data;  ///< data location
    };

    bool abort = false;             ///< set to force end of handling
    int block_timeout_ms = 10000;   ///< timeout between receiving blocks [ms]
    int read_timeout_ms = 2000;     ///< timeout for read after getting block header

protected:
    /// Read block data of expected size; return if successful
    virtual bool read_block(int32_t bsize);
    /// Set theblock to write point, or null if unavailable
    virtual void request_block(int32_t /*bsize*/) { if(!theblock) theblock = new dblock; }
    /// Return completed block to whence it came --- set to nullptr if not for re-use!
    virtual void return_block() { }

    /// Allocate block buffer space
    virtual char* alloc_block(int32_t bsize);
    /// Process data `theblock` after buffer read; return false to end communication
    virtual bool process(int32_t bsize);
    /// Process data after buffer read; return false to end communication
    virtual bool process_v(const vector<char>&);

    dblock* theblock = nullptr; ///< default buffer space
};

//////////////////////////////
//////////////////////////////

class SockBlockSerializerServer;

/// Block protocol handler for serializer server
class SockBlockSerializerHandler: public BlockHandler {
public:
    /// Constructor
    SockBlockSerializerHandler(int sfd, SockBlockSerializerServer* SBS);
protected:
    /// theblock = myServer->get_allocated()
    void request_block(int32_t /*bsize*/) override;
    /// myServer->return_allocated(theblock); theblock = nullptr
    void return_block() override;

    SockBlockSerializerServer* myServer;    ///< server handling serialization
};

#include "ThreadDataSerializer.hh"

/// Block data serializer server
class SockBlockSerializerServer: public ThreadedSockIOServer,
public ThreadDataSerializer<BlockHandler::dblock> {
protected:
    /// Generate handler that returns data to this
    ConnHandler* makeHandler(int sfd) override { return new SockBlockSerializerHandler(sfd, this); }
};

#endif
