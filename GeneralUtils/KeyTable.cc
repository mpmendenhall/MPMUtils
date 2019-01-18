/// \file KeyTable.cpp

#include "KeyTable.hh"

KeyData::KeyData(const KeyData& d): TMessage(d.What(), d.BufferSize()) {
    std::copy(d.Buffer(), d.Buffer()+d.BufferSize(), Buffer());
}

KeyData::KeyData(size_t n, const void* p): TMessage(kMESS_BINARY, sizeof(UInt_t) + n) {
    whut();
    WriteUInt(n);
    if(p) memcpy(fBufCur, p, n);
}

string KeyData::GetString() {
    if(whut() != kMESS_STRING) exit(8);
    UInt_t s = 0;
    ReadUInt(s);
    if(s > 10240) exit(8);
    string v(s, ' ');
    ReadFastArray(&v[0], v.size());
    return v;
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
