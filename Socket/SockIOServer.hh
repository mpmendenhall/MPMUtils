/// @file SockIOServer.hh I/O server for multiple socket connections

#ifndef SOCKIOSERVER_HH
#define SOCKIOSERVER_HH

#include "Threadworker.hh"
#include "SockConnection.hh"

class ConnHandler;

/// Base class listening and handling connections to port
class SockIOServer: public SockConnection, public ThreadManager {
public:
    /// receive and process connections to host and port
    void threadjob() override;

protected:
    /// handle each new connection; defaults to adding makeHandler() thread
    virtual void handle_connection(int csockfd);

    /// create correct handler type (runs in handle_connection() thread)
    virtual ConnHandler* makeHandler(int sfd);

    /// callback on connection handler close
    void on_thread_completed(Threadworker* t) override;
};

/// Base class for handling one accepted connection
class ConnHandler: public SockConnection, public Threadworker {
public:
    /// Constructor
    explicit ConnHandler(int sfd, SockIOServer* s):
    SockConnection(sfd), Threadworker(sfd, s) { }
    /// Communicate with accepted connection
    void threadjob() override;
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
    void threadjob() override;

    /// received data block with recipient identifier
    struct dblock {
        BlockHandler* H;    ///< pointer back to this handler
        vector<char> data;  ///< data location
    };

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

#endif
