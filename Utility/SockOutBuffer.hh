/// \file SockOutBuffer.hh Buffered socket data transfer

#ifndef SOCKOUTBUFFER_HH
#define SOCKOUTBUFFER_HH

#include "SockConnection.hh"
#include "LocklessCircleBuffer.hh"
using std::vector;

/// Buffered data block output to socket connection
class SockOutBuffer: public SockConnection, public LocklessCircleBuffer<vector<char>> {
public:
    /// Constructor
    explicit SockOutBuffer(size_t nbuff = 1000): LocklessCircleBuffer(nbuff) { }

    // use SockIOData* LocklessCircleBuffer::get_writepoint() and finish_write()
    // to push new data onto sending queue
    // use int launch_mythread(), finish_mythread() to send buffered data

protected:
    /// send data block
    void process_item() override;
};

#endif
