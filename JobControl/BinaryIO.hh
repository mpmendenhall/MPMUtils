/// \file BinaryIO.hh Base interface for serializing/sending/receiving/deserializing binary objects
// Michael P. Mendenhall, LLNL 2019

#ifndef BINARYIO_HH
#define BINARYIO_HH

#include <cassert>
#include <string>
using std::string;
#include <vector>
using std::vector;
#include <iostream>
#include <type_traits>

// workaround for older gcc without std::is_trivially_copyable
#if __GNUG__ && __GNUC__ < 5
#define IS_TRIVIALLY_COPYABLE(T) __has_trivial_copy(T)
#else
#define IS_TRIVIALLY_COPYABLE(T) std::is_trivially_copyable<T>::value
#endif

/// Base binary I/O class with serializer/deserializer functions
class BinaryIO {
public:
    /// blocking data send
    virtual void _send(void* vptr, int size) = 0;
    /// blocking data receive
    virtual void _receive(void* vptr, int size) = 0;

    /// generic data send
    template<typename T>
    void send(const T& v) {
        static_assert(IS_TRIVIALLY_COPYABLE(T), "Object needs custom send method");
        _send((void*)&v, sizeof(T));
    }

    /// tuple data send
    template<typename... T>
    void send(const std::tuple<T...>& t) { for(auto& c: t) send(c); }

    /// vector data send
    template<typename T>
    void send(const vector<T>& v) {
        send<int>(v.size()*sizeof(T));
        for(auto& x: v) send(x);
    }

    /// generic data receive
    template<typename T>
    void receive(T& v) {
        static_assert(IS_TRIVIALLY_COPYABLE(T), "Object needs custom receive method");
        _receive((void*)&v, sizeof(T));
    }
    /// generic data receive
    template<typename T>
    T receive() { T v; receive(v); return v; }

    /// tuple data receive
    template<typename... T>
    void receive(std::tuple<T...>& t) { for(auto& c: t) receive(c); }

    /// vector data receive
    template<typename T>
    void receive(vector<T>& v) { v.resize(receive<int>()/sizeof(T)); for(auto& x: v) receive(x); }
};

/// Binary I/O via iostream objects
class IOStreamBIO: public BinaryIO {
public:
    /// Constructor
    IOStreamBIO(std::istream* i, std::ostream* o): fIn(i), fOut(o) { }

    /// blocking data send
    void _send(void* vptr, int size) override { if(fOut) fOut->write((char*)vptr, size); }
    /// blocking data receive
    void _receive(void* vptr, int size) override { if(fIn) fIn->read((char*)vptr, size); }

    std::istream* fIn = nullptr;    ///< input stream
    std::ostream* fOut = nullptr;   ///< output stream
};

/// string data send
template<>
void BinaryIO::send<string>(const string& s);
/// Receive string
template<>
void BinaryIO::receive<string>(string& s);

#endif
