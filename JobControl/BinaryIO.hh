/// \file BinaryIO.hh Base interface for serializing/sending/receiving/deserializing binary objects
// Michael P. Mendenhall, LLNL 2019

#ifndef BINARYIO_HH
#define BINARYIO_HH

#include <cassert>
#include <type_traits>

#include <string>
using std::string;
#include <vector>
using std::vector;
#include <map>
using std::map;
#include <cstring> // for std::memcpy
#include <cassert>
#include <deque>
using std::deque;

// workaround for older gcc without std::is_trivially_copyable
#if __GNUG__ && __GNUC__ < 5
#define IS_TRIVIALLY_COPYABLE(T) __has_trivial_copy(T)
#else
#define IS_TRIVIALLY_COPYABLE(T) std::is_trivially_copyable<T>::value
#endif

/// Base binary class receiving input with serializer functions
class BinaryWriter {
public:
    /// Destructor
    virtual ~BinaryWriter() { }
    /// clear output, e.g. delete buffers
    virtual void clearOut() { }

    // optional: use to group writes together into a single transfer
    /// start buffered write transaction
    void start_wtx() { wtxdepth++; }
    /// end buffered write transaction
    void end_wtx();

    /// Dereference pointers by default
    template<typename T>
    void send(const T* p) { assert(p); send(*p); }

    /// data block send
    void send(const void* vptr, int size) {
         start_wtx();
         append_write((char*)vptr, size);
         end_wtx();
    }

    /// generic data send
    template<typename T, typename std::enable_if<!std::is_pointer<T>::value>::type* = nullptr>
    void send(const T& v) {
        static_assert(IS_TRIVIALLY_COPYABLE(T), "Object needs custom send method");
        send(&v, sizeof(T));
    }

    /// tuple data send
    template<typename... T>
    void send(const std::tuple<T...>& t) {
        start_wtx();
        for(auto& c: t) send(c);
        end_wtx();
    }

    /// vector data send
    template<typename T>
    void send(const vector<T>& v) {
        start_wtx();
        send<int>(v.size()*sizeof(T));
        for(auto& x: v) send(x);
        end_wtx();
    }

    /// map data send
    template<typename K, typename V>
    void send(const map<K,V>& mp) {
        start_wtx();
        send<size_t>(mp.size());
        for(auto& kt: mp) {
            send(kt.first);
            send(kt.second);
        }
        end_wtx();
    }

protected:

    /// blocking data send
    virtual void _send(void* vptr, int size) = 0;
    /// flush output
    virtual void flush() { }

    /// append data block to write buffer
    void append_write(const char* dat, size_t n);

    int dataDest = 0;       ///< destination identifier for data send
    size_t wtxdepth = 0;    ///< write transaction depth counter
    vector<char> wbuff;     ///< deferred write buffer
};

/// Base binary reader class with deserializer functions
class BinaryReader {
public:
    /// Destructor
    virtual ~BinaryReader() { }
    /// clear input
    virtual void clearIn() { }

    /// generic data receive
    template<typename T>
    void receive(T& v) {
        static_assert(IS_TRIVIALLY_COPYABLE(T), "Object needs custom receive method");
        _receive((void*)&v, sizeof(v));
    }
    /// out-of-place generic data receive
    template<typename T>
    T receive() { T v; receive(v); return v; }

    /// tuple data receive
    template<typename... T>
    void receive(std::tuple<T...>& t) { for(auto& c: t) receive(c); }

    /// vector data receive
    template<typename T>
    void receive(vector<T>& v) {
        v.resize(receive<int>()/sizeof(T));
        for(auto& x: v) receive(x);
    }

    /// map data receive
    template<typename K, typename V>
    void receive(map<K,V>& mp) {
        mp.clear();
        auto n = receive<size_t>();
        while(n--) {
            auto k = receive<K>();
            mp.emplace(k, receive<V>());
        }
    }

protected:

    /// blocking data receive
    virtual void _receive(void* vptr, int size) = 0;

    int dataSrc = 0;        ///< source identifier for data receive
    vector<char> rbuff;     ///< read buffer
    size_t rpt = 0;         ///< position in read buffer
};

/// Base combined binary I/O class
class BinaryIO: public BinaryReader, public BinaryWriter { };

/// string data send
template<>
void BinaryWriter::send<string>(const string& s);
/// Receive string
template<>
void BinaryReader::receive<string>(string& s);
/// treat const char* as string
template<>
inline void BinaryWriter::send(const char* x) { assert(x); send(string(x)); }

//////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////

/// Memory buffer BinaryReader
class MemBReader: public BinaryReader {
public:
    /// Constructor to non-owned buffer
    MemBReader(const void* p, size_t n): dR(p), pR(p), eR((const char*)p + n) { }

protected:
    /// blocking data receive
    void _receive(void* vptr, int size) override { assert((const char*)pR + size <= eR); std::memcpy(vptr,pR,size); (const char*&)pR += size; }

    const void* dR; ///< read data buffer
    const void* pR; ///< read position
    const void* eR; ///< end of read data buffer
};

/// Memory buffer BinaryWriter
class MemBWriter: public BinaryWriter {
public:
    /// Constructor to non-owned buffer
    MemBWriter(void* p, size_t n): dW(p), pW(p), eW((char*)p + n) { }

protected:
    /// blocking data send
    void _send(void* vptr, int size) override { assert((char*)pW + size <= eW); std::memcpy(pW,vptr,size); (char*&)pW += size; }

    void* dW;   ///< write data buffer
    void* pW;   ///< write position
    void* eW;   ///< end of write data buffer
};

/// Memory buffer I/O
class MemBIO: public MemBReader, public MemBWriter { };

/// I/O to a deque buffer; virtual to allow mix-in with BinaryIO inheritance
class DequeBIO: virtual public BinaryIO, protected deque<char> {
protected:
    /// blocking data send
    void _send(void* vptr, int s) override { auto v = (char*)vptr; while(s--) push_back(*(v++)); }
    /// blocking data receive
    void _receive(void* vptr, int s) override {
        if((int)size() < s) throw std::domain_error("Insufficient buffered data!");
        auto v = (char*)vptr;
        while(s--) { *(v++) = front(); pop_front(); }
    }
};

#endif
