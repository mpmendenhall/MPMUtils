/// @file SockBinIO.hh BinaryIO serialization/deserialization over buffered socket connection
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
    void _send(const void* vptr, int size) override {
        auto wp = sockfd? get_writepoint() : nullptr;
        if(!wp) return;
        auto vv = reinterpret_cast<const char*>(vptr);
        wp->assign(vv, vv + size);
        finish_write();
    }
};

/// Base binary reader class with deserializer functions
class SockBinRead: public BinaryReader, public SockFD {
public:
    /// Constructor
    explicit SockBinRead(int sfd = 0): SockFD(sfd) { }

protected:
    /// blocking data receive
    void _receive(void* vptr, int size) override { sockread(reinterpret_cast<char*>(vptr), size); }
};
