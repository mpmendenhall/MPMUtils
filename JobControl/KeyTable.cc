/// \file KeyTable.cpp

#include "KeyTable.hh"

template<>
KeyData::KeyData(const vector<double>& v): TMessage(kMESS_DOUBLE) {
    whut();
    send(v);
    SetReadMode();
}

KeyData& KeyData::operator=(const KeyData& d) {
    Expand(d.BufferSize());
    std::copy(d.Buffer(), d.Buffer()+d.BufferSize(), Buffer());
    SetReadMode();
    return *this;
}

KeyData::KeyData(size_t n, const void* p): TMessage(kMESS_BINARY, sizeof(UInt_t) + n) {
    whut();
    WriteUInt(n);
    if(p) memcpy(fBufCur, p, n);
    SetReadMode();
}

/////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////

KeyData* KeyTable::FindKey(const string& s, bool warn) const {
    auto it = find(s);
    if(it != end()) return it->second;
    if(warn) printf("KeyTable does not contain '%s'!\n", s.c_str());
    return nullptr;
}

bool KeyTable::Unset(const string& k) {
    auto it = find(k);
    if(it == end()) return false;
    delete it->second;
    erase(it);
    return true;
}

const KeyTable& KeyTable::operator=(const KeyTable& k) {
    if(&k == this) return *this;
    Clear();
    (map<string, KeyData*>&)(*this) = k;
    for(auto& kv: *this) kv.second = new KeyData(*kv.second);
    return *this;
}

bool KeyTable::_Set(const string& s, KeyData* v) {
    auto it = find(s);
    if(it != end()) {
        delete it->second;
        if(v) it->second = v;
        else erase(it);
        return true;
    } else if(v) (*this)[s] = v;
    return false;
}

/////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////

template<>
void BinaryIO::send<KeyData>(const KeyData& M) {
    start_wtx();
    send<UInt_t>(M.BufferSize());
    append_write(M.Buffer(), M.BufferSize());
    end_wtx();
}

template<>
KeyData* BinaryIO::receive<KeyData*>() {
    UInt_t s = receive<UInt_t>();
    auto buf = new char[s];
    _receive((void*)buf, s);
    return new KeyData(buf, s);
}

template<>
void BinaryIO::receive(KeyData& d) {
    UInt_t s = receive<UInt_t>();
    auto buf = new char[s];
    _receive((void*)buf, s);
    d = KeyData(buf, s);
}


