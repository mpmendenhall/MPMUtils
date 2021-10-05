/// \file SockBinIO.hh BinaryIO serialization/deserialization over buffered socket connection
// Michael P. Mendenhall, LLNL 2021

#include "BinaryIO.hh"
#include "SockOutBuffer.hh"

/// BinaryIO over buffered socket connection
class SockBinWrite: public BinaryWriter, public SockOutBuffer {
public:
    /// Inherit constructors
    using SockOutBuffer::SockOutBuffer;

protected:
    /// push data to socket buffer; drops if buffer full
    void _send(void* vptr, int size) override {
        auto wp = sockfd? get_writepoint() : nullptr;
        if(!wp) return;
        wp->assign((char*)vptr, (char*)(vptr) + size);
        finish_write();
    }
};

/// Base binary reader class with deserializer functions
class SockBinRead: public BinaryReader, public SockFD {
public:
    /// Constructor
    SockBinRead(int sfd = 0): SockFD(sfd) { }

protected:
    /// blocking data receive
    void _receive(void* vptr, int size) override { sockread((char*)vptr, size); }
};
