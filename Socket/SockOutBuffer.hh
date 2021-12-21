/// \file SockOutBuffer.hh Buffered socket data transfer

#ifndef SOCKOUTBUFFER_HH
#define SOCKOUTBUFFER_HH

#include "SockConnection.hh"
#include "LocklessCircleBuffer.hh"
using std::vector;

/// Buffered data block output to socket connection
class SockOutBuffer: public SockConnection, public LocklessCircleBuffer<vector<char>> {
public:
    /// inherit constructors
    using SockConnection::SockConnection;
    /// Destructor: finished buffered writes
    ~SockOutBuffer() { if(checkRunning()) finish_mythread(); }

    /// Establish output socket connection and start buffer pusher
    void connect_to_socket() override {
        SockConnection::connect_to_socket();
        launch_mythread();
    }
    /// avoid hiding alternate version
    using SockConnection::connect_to_socket;

    // set SocketConnection::host, port
    // use SockIOData* LocklessCircleBuffer::get_writepoint() and finish_write()
    // to push new data onto sending queue

protected:
    /// send data block
    void process_item() override;
};

#endif
