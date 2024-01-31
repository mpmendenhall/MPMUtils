/// \file MemBIO.hh Binary IO from in-memory buffers

#ifndef MEMBIO_HH
#define MEMBIO_HH

#include "BinaryIO.hh"

#include <deque>
using std::deque;

/// Memory buffer BinaryReader
class MemBReader: public BinaryReader {
public:
    /// Constructor to non-owned buffer
    explicit MemBReader(const void* p = nullptr, size_t n = 0) { setReadBuffer(p,n); }
    /// set buffer
    void setReadBuffer(const void* p, size_t n) { eR = pR = dR = reinterpret_cast<const char*>(p); eR += n; }

    /// skip over n bytes
    void ignore(size_t n) override;
    /// blocking data receive
    void read(void* vptr, size_t size) override;

protected:
    const char* dR = nullptr;   ///< read data buffer
    const char* pR = nullptr;   ///< read position
    const char* eR = nullptr;   ///< end of read data buffer
};

/// Memory buffer BinaryWriter
class MemBWriter: public BinaryWriter {
public:
    /// Constructor to non-owned buffer
    MemBWriter(void* p, size_t n): dW(reinterpret_cast<char*>(p)), pW(dW), eW(dW + n) { }

protected:
    /// blocking data send
    void _send(const void* vptr, size_t size) override;

    char* dW;   ///< write data buffer
    char* pW;   ///< write position
    char* eW;   ///< end of write data buffer
};

/// I/O to a deque buffer; virtual to allow mix-in with BinaryIO inheritance
class DequeBIO: virtual public BinaryIO, protected deque<char> {
protected:
    /// blocking data send
    void _send(const void* vptr, size_t s) override { auto v = reinterpret_cast<const char*>(vptr); while(s--) push_back(*(v++)); }
    /// blocking data receive
    void read(void* vptr, size_t s) override;
};

/// Buffering wrapper around another reader
class BufferingReader: public BinaryReader {
public:
    /// Constructor
    explicit BufferingReader(BinaryReader& _R, size_t b0 = 1024):
    R(_R), dchunk(b0) { }

    /// blocking data receive
    void read(void* vptr, size_t size) override;
    /// opportunistic data receive
    size_t read_upto(void* vptr, size_t size) override;

protected:
    /// reset buffer to start
    void rebuffer();
    /// fixed-length fetch from source appended to buffer end
    void load_buf(size_t s);
    /// opportunistic fetch from source appended to buffer end
    void load_buf_upto(size_t s);

    BinaryReader& R;        ///< wrapped BinaryReader
    size_t dchunk = 1024;   ///< read chunk size
    vector<char> dat;       ///< pre-read buffer data
    size_t rpos = 0;        ///< current position in read buffer
};

#endif
