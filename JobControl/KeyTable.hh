/// @file KeyTable.hh (string) key : (poymorphic) value table, wrapped with TMessage for serialized transfer
// -- Michael P. Mendenhall, LLNL 2019

#ifndef KEY_TABLE_H
#define KEY_TABLE_H

#include "BinaryIO.hh"
#include <TMessage.h>
#include <TObject.h>

#if BOOST_VERSION < 106900
#include <boost/functional/hash.hpp>
#else
#include <boost/container_hash/hash.hpp>
#endif

template<typename T>
struct _false: std::false_type { };

/// Polymorphic data value for KeyTable
class KeyData: public TMessage, protected BinaryIO {
public:
    // Buffer contains [UInt_t <unused message size>][UInt_t What][contents...]

    /// Polymorphic contents type information
    enum contents_t {
        kMESS_BINARY = 20000,       ///< generic binary blob
        kMESS_ARRAY  = 30000        ///< array type [][][UInt_t data size in bytes][data...]
    };

    /// identifiers for data types --- compose with kMESS_BINARY / kMESS_ARRAY
    template<typename T>
    static contents_t typeID(int i = 0) {
        i += std::min(sizeof(T), size_t(999));
        if(std::is_arithmetic<T>::value) {
            i += 1000;
            if(std::is_signed<T>::value) i += 2000;
            if(std::is_integral<T>::value) i += 4000;
        }
        return contents_t(i);
    }

    /// Default constructor
    KeyData(): TMessage(kMESS_ANY, 0) { std::memset(Buffer()+wsize, 0, BufferSize()-wsize); }
    /// Constructor, with 'what' and data size
    KeyData(int what, size_t n);
    /// Copy Constructor
    explicit KeyData(const KeyData& d): TMessage(d.What(), d.wSize()-2*sizeof(UInt_t)) { *this = d; }
    /// Constructor for string
    explicit KeyData(const string& s): TMessage(typeID<unsigned char>(kMESS_ARRAY), 0) { setData(s); }
    /// Copy operator
    KeyData& operator=(const KeyData& d);

    /// Constructor from TObject-derived classes
    template<typename T, typename std::enable_if<std::is_base_of<TObject, T>::value>::type* = nullptr>
    explicit KeyData(const T& o): TMessage(kMESS_OBJECT) {
        Reset();
        WriteObject(&o);
        wsize = fBufCur - Buffer();
        std::memset(fBufCur, 0, fBufMax-fBufCur);
        SetReadMode();
    }

    /// Constructor, writing generic (non-TObject) object
    template<typename T, typename std::enable_if<!std::is_base_of<TObject, T>::value>::type* = nullptr>
    explicit KeyData(const T& x): TMessage(typeID<T>(kMESS_BINARY),0) { setData(x); }

    /// Array types, with flags for numerical
    template<typename T>
    explicit KeyData(const vector<T>& v): TMessage(typeID<T>(kMESS_ARRAY), 0) { setData(v); }

    /// TObject-derived class value extraction (caller responsible for memory management of returned object)
    template<class C>
    C* GetROOT() {
        if(whut() != kMESS_OBJECT) throw std::runtime_error("Incorrect data type for ROOT object");
        Reset();
        return (C*)ReadObjectAny(C::Class());
    }

    /// Get data contents after headers
    char* data() { return Buffer() + 2*sizeof(UInt_t); }
    /// Const version of data
    const char* data() const { return Buffer() + 2*sizeof(UInt_t); }

    /// Get generic non-numeric type
    template<typename T, typename std::enable_if<!std::is_arithmetic<T>::value>::type* = nullptr>
    void Get(T& x) const { assert(What() != kMESS_OBJECT); MemBReader(data(), wsize-2*sizeof(UInt_t)).receive(x); }
    /// Get numeric type
    template<typename T, typename std::enable_if<std::is_arithmetic<T>::value>::type* = nullptr>
    void Get(T& x) const {
        auto w = What();
        if(w == typeID<T>(kMESS_BINARY)) x = *reinterpret_cast<const T*>(data());
        else {
            w -= kMESS_BINARY;
            /* */if(w == typeID<  int8_t>()) x = T(*reinterpret_cast<const int8_t*>(data()));
            else if(w == typeID< int16_t>()) x = T(*reinterpret_cast<const int16_t*>(data()));
            else if(w == typeID< int32_t>()) x = T(*reinterpret_cast<const int32_t*>(data()));
            else if(w == typeID< int64_t>()) x = T(*reinterpret_cast<const int64_t*>(data()));
            else if(w == typeID< uint8_t>()) x = T(*reinterpret_cast<const uint8_t*>(data()));
            else if(w == typeID<uint16_t>()) x = T(*reinterpret_cast<const uint16_t*>(data()));
            else if(w == typeID<uint32_t>()) x = T(*reinterpret_cast<const uint32_t*>(data()));
            else if(w == typeID<uint64_t>()) x = T(*reinterpret_cast<const uint64_t*>(data()));
            else if(w == typeID<   float>()) x = T(*reinterpret_cast<const float*>(data()));
            else if(w == typeID<  double>()) x = T(*reinterpret_cast<const double*>(data()));
            else if(w == typeID<long double>()) x = T(*reinterpret_cast<const long double*>(data()));
            else {
                std::stringstream ss;
                ss << "Unidentified numeric type " << w + kMESS_BINARY;
                throw std::domain_error(ss.str());
            }
        }
    }

    /// Get generic type out-of-place
    template<class T>
    T Get() const { T x{}; Get(x); return x; }

    /// Vector size for specified type
    template<typename T>
    UInt_t vSize() const {
        if(What() < kMESS_ARRAY) throw std::runtime_error("Incorrect data type for array");
        return (*reinterpret_cast<const int*>(data()))/sizeof(T);
    }
    /// Written data size [bytes]
    size_t wSize() const { assert((int)wsize <= BufferSize()); return wsize; }

    /// Retrieve pointer to start of non-ROOT data block
    template<typename T>
    T* GetArrayPtr() {
        assert(What() >= kMESS_ARRAY);
        return (T*)(data()+sizeof(UInt_t));
    }
    /// Retrieve pointer to start of non-ROOT data block
    template<typename T>
    const T* GetArrayPtr() const {
        if(What() < kMESS_ARRAY) throw std::logic_error("Not an array");
        return (const T*)(data()+sizeof(UInt_t));
    }

    /// contents sum operation, automatic for built-in arithmetic types and arrays thereof
    KeyData& operator+=(const KeyData& kd);

    /// Contents sum operation, automatically detecting array-of-contents
    template<typename T>
    void accumulate(const KeyData& kd) {
        if(What() < kMESS_ARRAY) {
            if(wsize != sizeof(T) + 2*sizeof(UInt_t)) throw std::logic_error("Invalid array data size");
            if(kd.wsize != wsize) throw std::domain_error("Incompatible data sizes!");
            *reinterpret_cast<T*>(data()) += *reinterpret_cast<const T*>(kd.data());
        } else {
            auto n = vSize<T>();
            if(n != kd.vSize<T>()) throw std::domain_error("Incompatible array sizes!");
            auto p0 = GetArrayPtr<T>();
            auto p1 = kd.GetArrayPtr<T>();
	    if(p0 == p1) throw std::logic_error("Accumulating into self");
            for(UInt_t i=0; i<n; i++) {
                if(*p0 == *p1) printf("%u: %g\n", i, double(*p0)); // shouldn't need this!! something else is broken!
                *(p0++) += *(p1++);
	    }
        }
    }

    /// clear array contents
    template<typename T = char>
    void clearV(const T c = {}) {
        auto n = vSize<T>();
        auto p0 = GetArrayPtr<T>();
        for(UInt_t i=0; i<n; i++) *(p0++) = c;
    }

    /// debugging display
    void display() const {
        auto w = What();
        printf("KeyData[%i: %zu/%i]", w, wsize, BufferSize());
        if(w/10000 == 2 && ((w/1000) & 1)) printf(" -> %g", Get<double>());
        printf("\n");
    }
    /// debugging binary display
    void bdisplay() const {
        display();
        printf("\t->");
        for(size_t i = 0; i < std::min(wsize,size_t(50)); i++) printf(" %2x", reinterpret_cast<const unsigned char*>(Buffer())[i]);
        printf("\n");
    }

    /// get What() stored in binary
    UInt_t _What() const { return *reinterpret_cast<const UInt_t*>(Buffer()+sizeof(UInt_t)); }

protected:
    /// Constructor, taking ownership of allocated buffer
    KeyData(void* buf, Int_t len): TMessage(buf, len) { SetReadMode(); wsize = len; }

    /// check type and set read point to beginning of contents
    UInt_t whut() { SetBufferOffset(sizeof(UInt_t)); UInt_t t = 0; ReadUInt(t); return t; }

    /// initialize to serialized contents; zero out extra space
    template<typename T>
    void setData(const T&x) { whut(); send(x); std::memset(fBufCur, 0, fBufMax-fBufCur); SetReadMode(); }

    /// append data to current write point
    void _send(const void* vptr, int size) override { WriteBuf(vptr, size); wsize = fBufCur - Buffer(); }
    /// pull data from current readpoint
    void _receive(void* vptr, int size) override { ReadBuf(vptr, size); }
    /// last _send data write size; includes first 2 bytes.
    size_t wsize = 2*sizeof(UInt_t);
};

namespace std {
    /// Hash function for KeyData binary contents
    template<>
    struct hash<KeyData> {
        size_t operator()(const KeyData& d) const noexcept {
            size_t n = d.wSize()-sizeof(UInt_t);
            const char* p = d.Buffer()+sizeof(UInt_t);
            return boost::hash_range(p, p+n);
        }
    };
}

/// string key : polymorphic value table
class KeyTable: public map<string, KeyData*> {
public:
    /// Default Constructor
    KeyTable() { }
    /// Copy constructor
    KeyTable(const KeyTable& other): map<string, KeyData*>() { *this = other; }
    /// Assignment operator
    KeyTable& operator=(const KeyTable& k);
    /// Destructor
    ~KeyTable() { Clear(); }

    /// Clear data
    void Clear() { for(const auto& kv: *this) delete kv.second; clear(); }

    /// Set from KeyData (taking ownership; delete entry if 'nullptr'); return 'true' if previous key deleted
    bool _Set(const string& k, KeyData* v);
    /// Set generic value for key
    template<typename T>
    bool Set(const string& k, const T& value) { return _Set(k, new KeyData(value)); }
    /// Remove value for key (return whether present)
    bool Unset(const string& k);

    /// Get value for key, or nullptr if undefined
    KeyData* FindKey(const string& k, bool warn = false) const;

    /// Get generic type; return whether found
    template<typename T>
    bool Get(const string& k, T& x, bool warn = false) const { auto v = FindKey(k,warn); if(v) v->Get(x); return v; }

    /// Get generic type out-of-place (required to exist)
    template<typename T>
    T Get(const string& k) const {
        T t{};
        auto v = FindKey(k,true);
        if(v) v->Get(t);
        else throw std::runtime_error("No such object: '"+k+"'");
        return t;
    }

    /// Get value with default
    template<typename T>
    T GetDefault(const string& k, T dflt) const { Get(k,dflt,false); return dflt; }

    /// Get TObject-derived class, or nullptr if key undefined or incorrect type
    template<class C>
    C* GetROOT(const string& k) const { auto v = FindKey(k, true); return v? v->GetROOT<C>() : nullptr; }

    /// Get modifiable pointer to array contents
    template<typename T>
    T* GetArrayPtr(const string& k) const { auto v = FindKey(k); return v? v->GetArrayPtr<T>() : nullptr; }

    /// debugging dump to stdout
    void display() const {
        printf("KeyTable with %zu entries\n", size());
        for(auto& kv: *this) {
            printf("\t* %s: ", kv.first.c_str());
            if(kv.second) kv.second->display();
            else printf("<NULL>\n");
        }
    }
};

/// Send KeyData buffered object
template<>
void BinaryWriter::send(const KeyData& M);
/// Receive (and accept memory management of) KeyData
template<>
void BinaryReader::receive(KeyData& d);
/// Receive (and accept memory management of) KeyData
template<>
KeyData* BinaryReader::receive<KeyData*>();

/// Send KeyTable
template<>
inline void BinaryWriter::send(const KeyTable& kt) { send((const map<string,KeyData*>&)kt); }
/// Receive KeyTable
template<>
inline void BinaryReader::receive(KeyTable& kt) { receive((map<string,KeyData*>&)kt); }

#endif
