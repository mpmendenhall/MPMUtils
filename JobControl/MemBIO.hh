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
    void ignore(size_t n) override {
        if(pR + n > eR) throw std::runtime_error("Invalid ignore quantity");
        pR += n;
    }

    /// blocking data receive
    size_t read(void* vptr, int size) override {
        if(pR + size > eR) throw std::runtime_error("Invalid receive allocation");
        std::memcpy(vptr,pR,size);
        pR += size;
        return size;
    }

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
    void _send(const void* vptr, int size) override {
        if(pW + size > eW) throw -1;
        std::memcpy(pW,vptr,size);
        pW += size;
    }

    char* dW;   ///< write data buffer
    char* pW;   ///< write position
    char* eW;   ///< end of write data buffer
};

/// I/O to a deque buffer; virtual to allow mix-in with BinaryIO inheritance
class DequeBIO: virtual public BinaryIO, protected deque<char> {
protected:
    /// blocking data send
    void _send(const void* vptr, int s) override { auto v = reinterpret_cast<const char*>(vptr); while(s--) push_back(*(v++)); }
    /// blocking data receive
    size_t read(void* vptr, int s) override {
        if((int)size() < s) throw std::domain_error("Insufficient buffered data!");
        auto s0 = s;
        auto v = reinterpret_cast<char*>(vptr);
        while(s--) { *(v++) = front(); pop_front(); }
        return s0;
    }
};

#endif
