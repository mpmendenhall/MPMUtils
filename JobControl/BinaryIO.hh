/// \file BinaryIO.hh Base interface for serializing/sending/receiving/deserializing binary objects
// Michael P. Mendenhall, LLNL 2019

#ifndef BINARYIO_HH
#define BINARYIO_HH

#include <cassert>
#include <string>
using std::string;
#include <string.h> // for memcpy
#include <vector>
using std::vector;
#include <iostream>
#include <type_traits>
#include <utility>
using std::pair;

// workaround for older gcc without std::is_trivially_copyable
#if __GNUG__ && __GNUC__ < 5
#define IS_TRIVIALLY_COPYABLE(T) __has_trivial_copy(T)
#else
#define IS_TRIVIALLY_COPYABLE(T) std::is_trivially_copyable<T>::value
#endif

/// Base binary I/O class with serializer/deserializer functions
class BinaryIO {
public:
    /// clear output
    virtual void clearOut() { }
    /// clear input
    virtual void clearIn() { }

    // optional: use to group writes together into a single transfer
    /// start buffered write transaction
    void start_wtx() { wtxdepth++; }
    /// end buffered write transaction
    void end_wtx();

    /// generic data send
    template<typename T>
    void send(const T& v) {
        static_assert(IS_TRIVIALLY_COPYABLE(T), "Object needs custom send method");
        start_wtx();
        append_write((char*)&v, sizeof(T));
        end_wtx();
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

    /// out-of-place generic data receive
    template<typename T>
    T receive() { T v; receive(v); return v; }

    /// generic data receive
    template<typename T>
    void receive(T& v) {
        static_assert(IS_TRIVIALLY_COPYABLE(T), "Object needs custom receive method");
        _receive((void*)&v, sizeof(v));
    }

    /// tuple data receive
    template<typename... T>
    void receive(std::tuple<T...>& t) { for(auto& c: t) receive(c); }

    /// vector data receive
    template<typename T>
    void receive(vector<T>& v) {
        v.resize(receive<int>()/sizeof(T));
        for(auto& x: v) receive(x);
    }

protected:

    /// blocking data send
    virtual void _send(void* vptr, int size) = 0;
    /// blocking data receive
    virtual void _receive(void* vptr, int size) = 0;
    /// flush output
    virtual void flush() { }

    /// append data block to write buffer
    void append_write(const char* dat, size_t n);

    int dataSrc = 0;        ///< source for data receive
    int dataDest = 0;       ///< destination for data send
    size_t wtxdepth = 0;    ///< write transaction depth counter
    vector<char> wbuff;     ///< deferred write buffer
    vector<char> rbuff;     ///< read buffer
    size_t rpt = 0;         ///< position in read buffer
};

/// string data send
template<>
void BinaryIO::send<string>(const string& s);
/// Receive string
template<>
void BinaryIO::receive<string>(string& s);

#endif
