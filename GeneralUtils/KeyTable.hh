/// \file KeyTable.h (string) key : (poymorphic) value table, wrapped with TMessage for serialized transfer
// Michael P. Mendenhall, LLNL 2019

#ifndef KEY_TABLE_H
#define KEY_TABLE_H

#include <TMessage.h>
#include <TObject.h>
#include <map>
#include <string>
#include <vector>
using namespace std;

/// Polymorphic data value for KeyTable
class KeyData: public TMessage {
public:
    /// Polymorphic contents type information
    enum contents_t {
        kMESS_BINARY = 20000,   ///< generic binary blob (UInt_t size, char* data[size])
        kMESS_DOUBLE = 20001,   ///< array of double
        kMESS_STRING = 20002    ///< "string" array of char
    };

    /// Copy Constructor
    KeyData(const KeyData& d);
    /// Constructor, taking ownership of allocated buffer
    KeyData(void* buf, Int_t len): TMessage(buf, len) { }

    /// Constructor, holding ROOT object
    KeyData(const TObject* o): TMessage(kMESS_OBJECT) { if(!o) exit(1); Reset(); WriteObject(o); SetReadMode(); }
    /// Constructor, holding vector of structs
    template<typename T>
    KeyData(const vector<T>& v): TMessage(std::is_same<T,double>::value? kMESS_DOUBLE : kMESS_BINARY, sizeof(UInt_t) + v.size()*sizeof(T)) {
        whut();
        WriteUInt(v.size()*sizeof(T));
        memcpy(fBufCur, v.data(), v.size()*sizeof(T));
    }
    /// Constructor, holding string
    KeyData(const string& v): TMessage(kMESS_STRING, sizeof(UInt_t) + v.size()*sizeof(char)) { whut(); WriteArray(v.c_str(), v.size()); }
    /// Constructor, holding arbitrary binary blob
    KeyData(size_t n, const void* p);

    /// check type and set read point to beginning of contents
    UInt_t whut() { SetBufferOffset(sizeof(UInt_t)); UInt_t t = 0; ReadUInt(t); return t; }

    /// TObject-derived class value extraction (new object not memory managed by or referring to this)
    template<class C>
    C* GetROOT() { C* o = nullptr; if(whut() == kMESS_OBJECT) { o = (C*)ReadObjectAny(C::Class()); if(o) o = (C*)o->Clone(); } return  o; }

    /// Vector size for specified type
    template<typename T>
    UInt_t vSize() {
        if(whut() < kMESS_BINARY) exit(2);
        UInt_t sz = 0;
        ReadUInt(sz);
        return sz/sizeof(T);
    }
    /// Retrieve string contents
    string GetString();
    /// Retrieve (re-interpreted) binary struct
    template<typename T>
    T& GetStruct() {
        if(whut() < kMESS_BINARY) exit(3);
        UInt_t tt = 0;
        ReadUInt(tt);
        if(tt != sizeof(T)) exit(4);
        return *(T*)fBufCur;
    }
    /// Retrieve pointer to start of non-ROOT data block
    template<typename T>
    T* GetPtr() {
        vSize<T>();
        return (T*)fBufCur;
    }
    /// Extract (non-reference) vector of items
    template<typename T>
    vector<T> GetVector() {
        vector<T> v(vSize<T>());
        auto p = (T*)fBufCur;
        for(auto& x: v) x = *(p++);
        return v;
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

    /// All numeric types converted to double by default
    template<typename T, typename std::enable_if<std::is_arithmetic<T>::value>::type* = nullptr>
    bool Set(const string& k, const T& value) { return SetStruct<double>(k, (double)value); }
    /// Catchall for non-numeric types
    template<typename T, typename std::enable_if<!std::is_arithmetic<T>::value>::type* = nullptr>
    bool Set(const string& k, const T& value) { return _Set(k, new KeyData(value)); }

    template<typename T>
    bool SetStruct(const string& k, const T& value) { return _Set(k, new KeyData(sizeof(T), &value)); }

    /// Get value for key, or nullptr if undefined
    KeyData* FindKey(const string& k, bool warn = false) const;
    /// Remove value for key (return whether present)
    bool Unset(const string& k);

    /// Get TObject-derived class, or nullptr if key undefined or incorrect type
    template<class C>
    C* GetROOT(const string& k) const { auto v = FindKey(k); return v? v->GetROOT<C>() : nullptr; }
    /// Get modifiable pointer to array contents
    template<typename T>
    T* GetPtr(const string& k) const { auto v = FindKey(k); return v? v->GetPtr<T>() : nullptr; }
    /// Get vector for key
    template<typename T>
    vector<T> GetVector(const string& k) const { auto v = FindKey(k,true); return v? v->GetVector<T>() : vector<T>(); }
    /// Get string for key
    string GetString(const string& k) const { auto v = FindKey(k,true); return v? v->GetString() : k+"-NOTFOUND"; }
    /// Get sruct-type value
    template<typename T>
    T& GetStruct(const string& k, bool warn = false) const { auto v = FindKey(k,warn); if(!v) { exit(1); } return v->GetStruct<T>(); }
    /// Get double value for key
    double& GetDouble(const string& k) const { return GetStruct<double>(k,true); }
    /// Get double if available
    void Get(const string& k, double& x) const { auto v = FindKey(k); if(v) x = v->GetStruct<double>(); }
    /// Get string if available
    void Get(const string& k, string& s) const { auto v = FindKey(k); if(v) s = v->GetString(); }
    /// Get int (from double) if available
    void Get(const string& k, int& i) const { auto v = FindKey(k); if(v) i = v->GetStruct<double>(); }
    /// Get bool (from double) if available
    void Get(const string& k, bool& b) const { auto v = FindKey(k); if(v) b = v->GetStruct<double>(); }
    /// Get boolean with default
    bool GetBool(const string& k, bool dflt = false) { Get(k,dflt); return dflt; }
};

#endif
