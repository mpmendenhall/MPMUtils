/// \file KeyTable.hh (string) key : (poymorphic) value table, wrapped with TMessage for serialized transfer
// Michael P. Mendenhall, LLNL 2019

#ifndef KEY_TABLE_H
#define KEY_TABLE_H

#include "BinaryIO.hh"
#include <TMessage.h>
#include <TObject.h>
#include <map>
using std::map;
#include <stdexcept>

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
    static constexpr contents_t typeID(int i = 0) {
        i += std::min(sizeof(T), size_t(999));
        if(std::is_arithmetic<T>::value) {
            i += 1000;
            if(std::is_signed<T>::value) i += 2000;
            if(std::is_integral<T>::value) i += 4000;
        }
        return contents_t(i);
    }

    /// Default Constructor
    KeyData(): TMessage(kMESS_ANY, 0) { wsize = 2*sizeof(UInt_t); SetReadMode(); }
    /// Constructor, taking ownership of allocated buffer
    KeyData(void* buf, Int_t len): TMessage(buf, len) { SetReadMode(); wsize = len; }
    /// Constructor, copied from binary blob (or nullptr for empty buffer)
    KeyData(size_t n, const void* p);
    /// Constructor, with 'what' and data size
    KeyData(int what, size_t n): TMessage(what,n) { wsize = 2*sizeof(UInt_t)+n; SetReadMode(); }

    /// Copy Constructor
    KeyData(const KeyData& d): TMessage(d.What(), d.wSize()-2*sizeof(UInt_t)) { *this = d; }
    /// Copy operator
    KeyData& operator=(const KeyData& d);

    /// Constructor, writing generic non-ROOT object
    template<typename T, typename std::enable_if<!std::is_base_of<TObject, T>::value>::type* = nullptr>
    KeyData(const T& x): TMessage(typeID<T>(kMESS_BINARY),0) { setData(x); }

    /// Array types, with flags for numerical
    template<typename T>
    KeyData(const vector<T>& v): TMessage(typeID<T>(kMESS_ARRAY), 0) { setData(v); }
    /// Constructor for string
    KeyData(const string& s): TMessage(typeID<unsigned char>(kMESS_ARRAY), 0) { setData(s); }

    /// Constructor, writing ROOT object
    template<typename T, typename std::enable_if<std::is_base_of<TObject, T>::value>::type* = nullptr>
    KeyData(const T& o): TMessage(kMESS_OBJECT) { Reset(); WriteObject(&o); wsize = fBufCur - Buffer(); SetReadMode(); }

    /// TObject-derived class value extraction (new object not memory managed by or referring to this)
    template<class C>
    C* GetROOT() {
        if(whut() != kMESS_OBJECT) throw std::runtime_error("Incorrect data type for ROOT object");
        C* o = (C*)ReadObjectAny(C::Class());
        if(o) o = (C*)o->Clone((o->GetName() + string("_b")).c_str());
        return  o;
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
        if(w == typeID<T>(kMESS_BINARY)) x = *(T*)data();
        else {
            w -= kMESS_BINARY;
            /* */if(w == typeID<  int8_t>()) x = T(*(int8_t*)data());
            else if(w == typeID< int16_t>()) x = T(*(int16_t*)data());
            else if(w == typeID< int32_t>()) x = T(*(int32_t*)data());
            else if(w == typeID< int64_t>()) x = T(*(int64_t*)data());
            else if(w == typeID< uint8_t>()) x = T(*(uint8_t*)data());
            else if(w == typeID<uint16_t>()) x = T(*(uint16_t*)data());
            else if(w == typeID<uint32_t>()) x = T(*(uint32_t*)data());
            else if(w == typeID<uint64_t>()) x = T(*(uint64_t*)data());
            else if(w == typeID<   float>()) x = T(*(float*)data());
            else if(w == typeID<  double>()) x = T(*(double*)data());
            else if(w == typeID<long double>()) x = T(*(long double*)data());
            else throw std::domain_error("Unidentified numeric type!");

        }
    }

    /// Get generic type out-of-place
    template<class T>
    T Get() const { T x{}; Get(x); return x; }

    /// Vector size for specified type
    template<typename T>
    UInt_t vSize() const {
        if(What() < kMESS_ARRAY) throw std::runtime_error("Incorrect data type for array");
        return (*(int*)data())/sizeof(T);
    }
    /// Written data size [bytes]
    size_t wSize() const { assert((int)wsize <= BufferSize()); return wsize; }

    /// Retrieve pointer to start of non-ROOT data block
    template<typename T>
    T* GetArrayPtr() {
        assert(What() >= kMESS_ARRAY);
        return (T*)(data()+sizeof(int));
    }
    /// Retrieve pointer to start of non-ROOT data block
    template<typename T>
    const T* GetArrayPtr() const {
        assert(What() >= kMESS_ARRAY);
        return (const T*)(data()+sizeof(int));
    }

    /// contents sum operation, automatic for built-in arithmetic types and arrays thereof
    void accumulate(const KeyData& kd);

    /// Contents sum operation, automatically detecting array-of-contents
    template<typename T>
    void accumulate(const KeyData& kd) {
        if(What() < kMESS_ARRAY) {
            assert(wsize == sizeof(T)+2*sizeof(UInt_t));
            if(kd.wsize != wsize) throw std::domain_error("Incompatible data sizes!");
            *(T*)data() += *(T*)kd.data();
        } else {
            auto n = vSize<T>();
            if(n != kd.vSize<T>()) throw std::domain_error("Incompatible array sizes!");
            auto p0 = GetArrayPtr<T>();
            auto p1 = kd.GetArrayPtr<T>();
            for(UInt_t i=0; i<n; i++) *(p0++) += *(p1++);
        }
    }

    /// clear array contents
    template<typename T = char>
    void clearV(const T c = {}) {
        auto n = vSize<T>();
        auto p0 = (T*)fBufCur;
        for(UInt_t i=0; i<n; i++) *(p0++) = c;
    }

    /// debugging binary display
    void bdisplay() const {
        printf("KD[%zu]\t", wsize);
        for(size_t i = 0; i < std::min(wsize,size_t(50)); i++) printf(" %2x", ((const unsigned char*)Buffer())[i]);
        printf("\n");
    }

    /// get What() stored in binary
    UInt_t _What() const { return *(UInt_t*)(Buffer()+sizeof(UInt_t)); }

protected:
    /// check type and set read point to beginning of contents
    UInt_t whut() { SetBufferOffset(sizeof(UInt_t)); UInt_t t = 0; ReadUInt(t); return t; }
    /// set contents; zero out extra space
    template<typename T>
    void setData(const T&x) { whut(); send(x); std::memset(fBufCur, 0, fBufMax-fBufCur); SetReadMode(); }
    /// append data to current write point
    void _send(void* vptr, int size) override { WriteBuf(vptr, size); wsize = fBufCur - Buffer(); }
    /// pull data from current readpoint
    void _receive(void* vptr, int size) override { ReadBuf(vptr, size); }
    /// last _send data write size
    size_t wsize = 0;
};

/// Hash function for KeyData binary contents
template<>
struct std::hash<KeyData> {
    size_t operator()(const KeyData& d) const noexcept {
        size_t n = d.wSize()-sizeof(UInt_t);
        const char* p = d.Buffer()+sizeof(UInt_t);
        return boost::hash_range(p, p+n);
    }
};

/// string key : polymorphic value table
class KeyTable: public map<string, KeyData*> {
public:
    /// Default Constructor
    KeyTable() { }
    /// Copy constructor
    KeyTable(const KeyTable& other): map<string, KeyData*>() { *this = other; }
    /// Assignment operator
    const KeyTable& operator=(const KeyTable& k);
    /// Destructor
    ~KeyTable() { Clear(); }

    /// Clear data
    void Clear() { for(auto& kv: *this) delete kv.second; clear(); }

    /// Set from KeyData (taking ownership; delete entry if 'nullptr'); return 'true' if previous key deleted
    bool _Set(const string& k, KeyData* v);

    /// bool stored as int...
    bool Set(const string& k, bool value) { return Set(k, int(value)); }

    /// Set generic value for key
    template<typename T>
    bool Set(const string& k, const T& value) { return _Set(k, new KeyData(value)); }

    /// Get value for key, or nullptr if undefined
    KeyData* FindKey(const string& k, bool warn = false) const;
    /// Remove value for key (return whether present)
    bool Unset(const string& k);

    /// Get TObject-derived class, or nullptr if key undefined or incorrect type
    template<class C>
    C* GetROOT(const string& k) const { auto v = FindKey(k, true); return v? v->GetROOT<C>() : nullptr; }
    /// Get modifiable pointer to array contents
    template<typename T>
    T* GetArrayPtr(const string& k) const { auto v = FindKey(k); return v? v->GetArrayPtr<T>() : nullptr; }

    /// Get boolean stored as int
    void Get(const string& k, bool& b, bool warn = false) { int i = b; Get(k, i, warn); b = i; }
    /// Get generic type
    template<typename T>
    void Get(const string& k, T& x, bool warn = false) const { auto v = FindKey(k,warn); if(v) v->Get(x); }
    /// Get generic type out-of-place (required to exist)
    template<typename T>
    T Get(const string& k) const { T t{}; auto v = FindKey(k,true); if(v) v->Get(t); else assert(false); return t; }

    /// Get boolean (stored as int stored as double) with default
    bool GetBool(const string& k, bool dflt = false) const { int i = dflt; Get(k,i,false); return i; }
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
