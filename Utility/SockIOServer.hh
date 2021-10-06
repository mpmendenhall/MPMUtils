/// \file SockIOServer.hh I/O server for multiple socket connections

#ifndef SOCKIOSERVER_HH
#define SOCKIOSERVER_HH

#include "ThreadDataSerializer.hh"
#include "SockConnection.hh"

#include <string>
using std::string;
#include <set>
using std::set;
using std::vector;

class ConnHandler;

/// Base class listening and handling connections to port
class SockIOServer: public SockConnection, public Threadworker {
public:
    /// receive and process connections to host and port
    void threadjob() override;

protected:
    /// handle each new connection --- subclass me!
    virtual void handle_connection(int csockfd);
};

/// Base class for handling one accepted connection
class ConnHandler: public SockConnection, public Threadworker {
public:
    /// Constructor
    explicit ConnHandler(int sfd, SockIOServer* s = nullptr): SockConnection(sfd), myServer(s) { }
    /// Communicate with accepted connection
    virtual void handle();
    /// register with server, run handle(), delete this
    void threadjob() override;

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
    mutex connsLock;       ///< lock on myConns
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
    /// Allocate block buffer space (default: allocate in theblock->data)
    virtual char* alloc_block(int32_t bsize);
    /// Set theblock to write point, or null if unavailable
    virtual void request_block(int32_t /*bsize*/) { if(!theblock) theblock = new dblock; }
    /// Return completed block to whence it came --- set to nullptr if not for re-use!
    virtual void return_block() { }

    /// Process data after buffer read (default: calls process_v on theblock->data); return false to end communication
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
    /// theblock = mySBSS->get_allocated()
    void request_block(int32_t /*bsize*/) override;
    /// mySBSS->return_allocated(theblock); theblock = nullptr
    void return_block() override;

    SockBlockSerializerServer* mySBSS;  ///< server handling serialization
};

/// Block data serializer server
class SockBlockSerializerServer: public ThreadedSockIOServer,
public ThreadDataSerializer<BlockHandler::dblock> {
protected:
    /// Generate handler that returns data to this
    ConnHandler* makeHandler(int sfd) override { return new SockBlockSerializerHandler(sfd, this); }
};

#endif
