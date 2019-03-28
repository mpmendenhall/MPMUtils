/// \file KeyTable.hh (string) key : (poymorphic) value table, wrapped with TMessage for serialized transfer
// Michael P. Mendenhall, LLNL 2019

#ifndef KEY_TABLE_H
#define KEY_TABLE_H

#include "BinaryIO.hh"
#include <TMessage.h>
#include <TObject.h>
#include <map>
using std::map;

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
    /// Polymorphic contents type information
    enum contents_t {
        kMESS_BINARY = 20000,   ///< generic binary blob (UInt_t size, char* data[size])
        kMESS_DOUBLE = 20001,   ///< array of double
        kMESS_STRING = 20002    ///< "string" array of char
    };

    /// Default Constructor
    KeyData(): TMessage(0) { }
    /// Copy Constructor
    KeyData(const KeyData& d): TMessage(d.What()) { *this = d; }
    /// Copy operator
    KeyData& operator=(const KeyData& d);

    /// Constructor, taking ownership of allocated buffer
    KeyData(void* buf, Int_t len): TMessage(buf, len) { SetReadMode(); }

    /// Constructor, holding arbitrary binary blob
    KeyData(size_t n, const void* p);

    /// Constructor, writing generic non-ROOT object
    template<typename T, typename std::enable_if<!std::is_base_of<TObject, T>::value>::type* = nullptr>
    KeyData(const T& x): TMessage(kMESS_BINARY,0) { whut(); std::memset(fBufCur,0,fBufMax-fBufCur); send(x); SetReadMode(); }
    /// Constructor, writing ROOT object
    template<typename T, typename std::enable_if<std::is_base_of<TObject, T>::value>::type* = nullptr>
    KeyData(const T& o): TMessage(kMESS_OBJECT) { Reset(); WriteObject(&o); SetReadMode(); }

    /// check type and set read point to beginning of contents
    UInt_t whut() { SetBufferOffset(sizeof(UInt_t)); UInt_t t = 0; ReadUInt(t); return t; }

    /// TObject-derived class value extraction (new object not memory managed by or referring to this)
    template<class C>
    C* GetROOT() {
        if(whut() != kMESS_OBJECT) exit(20);
        C* o = (C*)ReadObjectAny(C::Class());
        if(o) o = (C*)o->Clone((o->GetName() + string("_b")).c_str());
        return  o;
    }

    /// Get generic type
    template<typename T>
    void Get(T& x) { whut(); receive(x); }
    /// Get generic type
    template<class T>
    T Get() { T x; Get(x); return x; }

    /// Vector size for specified type
    template<typename T>
    UInt_t vSize() {
        if(whut() < kMESS_BINARY) exit(2);
        return receive<int>()/sizeof(T);
    }

    /// Retrieve pointer to start of non-ROOT data block
    template<typename T>
    T* GetPtr() {
        vSize<T>();
        return (T*)fBufCur;
    }

    /// contents sum operation
    template<typename T>
    void accumulate(KeyData& kd) {
        auto n = vSize<T>();
        if(n != kd.vSize<T>()) exit(5);
        auto p0 = (T*)fBufCur;
        auto p1 = (T*)kd.fBufCur;
        for(UInt_t i=0; i<n; i++) *(p0++) += *(p1++);
    }

    /// clear array contents
    template<typename T>
    void clear(const T c = 0) {
        auto n = vSize<T>();
        auto p0 = (T*)fBufCur;
        for(UInt_t i=0; i<n; i++) *(p0++) = c;
    }

protected:
    /// append data to current write point
    void _send(void* vptr, int size) override { WriteBuf(vptr, size); }
    /// pull data from current readpoint
    void _receive(void* vptr, int size) override { ReadBuf(vptr, size); }
};

template<>
KeyData::KeyData(const vector<double>& v);

template<>
struct std::hash<KeyData> {
    size_t operator()(const KeyData& d) const noexcept {
        size_t n = d.BufferSize();
        const char* p = d.Buffer();
        return boost::hash_range(p, p+n);
    }
};

/// string key : polymorphic value table
class KeyTable: public map<string, KeyData*> {
public:
    /// Constructor.
    KeyTable() { }
    /// Destructor
    ~KeyTable() { Clear(); }
    /// Clear data
    void Clear() { for(auto& kv: *this) delete kv.second; clear(); }
    /// Check for key.
    bool HasKey(const string& k) { return count(k); }
    /// Copy constructor
    KeyTable(const KeyTable& other): map<string, KeyData*>() { *this = other; }
    /// Assignment operator
    const KeyTable& operator=(const KeyTable& k);

    /// Set from KeyData (taking ownership; delete entry if 'nullptr'); return 'true' if previous key deleted
    bool _Set(const string& k, KeyData* v);

    /// bool stored as int stored as double...
    bool Set(const string& k, bool value) { return Set(k, int(value)); }
    /// All numeric types converted to double by default
    template<typename T, typename std::enable_if<std::is_arithmetic<T>::value>::type* = nullptr>
    bool Set(const string& k, const T& value) { return _Set(k, new KeyData(double(value))); }
    /// Catchall for non-numeric types
    template<typename T, typename std::enable_if<!std::is_arithmetic<T>::value>::type* = nullptr>
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
    T* GetPtr(const string& k) const { auto v = FindKey(k); return v? v->GetPtr<T>() : nullptr; }

    /// Get boolean stored as int
    void Get(const string& k, bool& b, bool warn = false) { int i = b; Get(k, i, warn); b = i; }
    /// All numeric types converted to double by default; use, e.g., GetT<int>() for non-doubles
    template<typename T, typename std::enable_if<std::is_arithmetic<T>::value>::type* = nullptr>
    void Get(const string& k, T& x, bool warn = false) {  auto v = FindKey(k,warn); if(v) { double y = x; v->Get(y); x = y; } }
    /// Get non-numeric if available
    template<class C, typename std::enable_if<!std::is_arithmetic<C>::value>::type* = nullptr>
    void Get(const string& k, C& x, bool warn = false) const { auto v = FindKey(k,warn); if(v) v->Get(x); }

    /// Get generic if available (no autoconversion to double)
    template<typename T>
    T GetT(const string& k, bool warn = false) const { T t; auto v = FindKey(k,warn); if(v) v->Get(t); return t; }

    /// Get boolean (stored as int stored as double) with default
    bool GetBool(const string& k, bool dflt = false) { int i = dflt; Get(k,i,false); return i; }
};

/// Send KeyData buffered object
template<>
void BinaryIO::send(const KeyData& M);
/// Receive (and accept memory management of) KeyData
template<>
void BinaryIO::receive(KeyData& d);
/// Receive (and accept memory management of) KeyData
template<>
KeyData* BinaryIO::receive<KeyData*>();

/// Send KeyTable
template<>
inline void BinaryIO::send(const KeyTable& kt) { send((const map<string,KeyData*>&)kt); }
/// Receive KeyTable
template<>
inline void BinaryIO::receive(KeyTable& kt) { receive((map<string,KeyData*>&)kt); }

#endif
