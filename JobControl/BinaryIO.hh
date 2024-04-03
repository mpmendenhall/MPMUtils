/// @file BinaryIO.hh Base interface for serializing/sending/receiving/deserializing binary objects
// -- Michael P. Mendenhall, LLNL 2019

#ifndef BINARYIO_HH
#define BINARYIO_HH

#include <type_traits>

#include <string>
using std::string;
#include <vector>
using std::vector;
#include <map>
using std::map;
#include <cstring> // for std::memcpy
#include <stdexcept>

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

    /// chainable readin operator
    template<typename T>
    BinaryWriter& operator<<(const T& v) { send(v); return *this; }

    /// unhappy sending pointers
    template<typename T>
    void send(const T*) { throw std::logic_error("Don't send pointers"); }

    /// data block send
    void send(const void* vptr, size_t size) {
         start_wtx();
         append_write(reinterpret_cast<const char*>(vptr), size);
         end_wtx();
    }
    /// raw binary vector send
    template<typename T>
    void sendblock(const vector<T>& dat) { send(reinterpret_cast<const void*>(dat.data()), dat.size()*sizeof(T)); }

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
        for(const auto& c: t) send(c);
        end_wtx();
    }

    /// vector data send
    template<typename T>
    void send(const vector<T>& v) {
        start_wtx();
        send<int>(v.size()*sizeof(T));
        for(const auto& x: v) send(x);
        end_wtx();
    }

    /// map data send
    template<typename K, typename V>
    void send(const map<K,V>& mp) {
        start_wtx();
        send<size_t>(mp.size());
        for(const auto& kt: mp) {
            send(kt.first);
            send(kt.second);
        }
        end_wtx();
    }

protected:

    /// blocking data send
    virtual void _send(const void* vptr, size_t size) = 0;
    /// flush output
    virtual void flush() { }

    /// append data block to write buffer
    void append_write(const char* dat, size_t n);

    int dataDest = 0;       ///< destination identifier for data send
    size_t wtxdepth = 0;    ///< write transaction depth counter
    vector<char> wbuff;     ///< deferred write buffer
};

/// scope-guarded transaction
class WriteScope {
public:
    /// Constructor
    explicit WriteScope(BinaryWriter& _B): B(_B) { B.start_wtx(); }
    /// Destructor
    ~WriteScope() { B.end_wtx(); }
private:
    BinaryWriter& B;
};

/// Binary writer with exposed buffer for serialization
class BinarySerializer: public BinaryWriter {
public:
    /// Constructor; starts at wtxDepth 1 to delay buffer clear
    BinarySerializer() { start_wtx(); }
    /// direct buffer access
    vector<char>& buf() { return wbuff; }

protected:
    /// _send does nothing!
    void _send(const void*, size_t) override { }
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
        read(static_cast<void*>(&v), sizeof(v));
    }
    /// out-of-place generic data receive
    template<typename T>
    T receive() { T v; receive(v); return v; }
    /// chainable readout operator
    template<typename T>
    BinaryReader& operator>>(T& v) { receive(v); return *this; }

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

    /// raw blocking data receive; error if full read not achieved
    virtual void read(void* vptr, size_t size) = 0;
    /// non-blocking opportunistic read of all available to size
    virtual size_t read_upto(void*, size_t) { return 0; }
    /// skip over n bytes (... please reimplement faster!)
    virtual void ignore(size_t n) { vector<char> foo(n); read(foo.data(), n); }

protected:
    int dataSrc = 0;    ///< source identifier for data receive

};

/// string data send
template<>
void BinaryWriter::send<string>(const string& s);

/// Receive string
template<>
void BinaryReader::receive<string>(string& s);

/// treat const char* as string
template<>
inline void BinaryWriter::send(const char* x) { send(string(x)); }

#endif
