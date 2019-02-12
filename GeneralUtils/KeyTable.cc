/// \file KeyTable.cpp

#include "KeyTable.hh"

template<>
KeyData::KeyData(const char* x): TMessage(kMESS_STRING) {
    string s(x);
    whut();
    send(s);
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
    send<UInt_t>(M.BufferSize());
    _send(M.Buffer(), M.BufferSize());
}

template<>
KeyData* BinaryIO::receive<KeyData*>() {
    auto s = receive<UInt_t>();
    if(!s) return nullptr;

    auto buf = new char[s];
    _receive(buf, s);
    return new KeyData(buf, s);
}

template<>
void BinaryIO::receive(KeyData& d) {
    auto s = receive<UInt_t>();
    auto buf = new char[s];
    _receive(buf, s);
    d = KeyData(buf, s);
}

template<>
void BinaryIO::send(const KeyTable& kt) {
    send<size_t>(kt.size());
    for(auto& kv: kt) {
        send(kv.first);
        send(*kv.second);
    }
}

template<>
void BinaryIO::receive(KeyTable& kt) {
    kt.Clear();
    auto ktSize = receive<size_t>();
    while(ktSize--) {
        auto kn = receive<string>();
        kt._Set(kn, receive<KeyData*>());
    }
}
