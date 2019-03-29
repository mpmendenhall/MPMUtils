/// \file KeyTable.cc

#include "KeyTable.hh"

KeyData& KeyData::operator=(const KeyData& d) {
    wsize = d.wSize();
    Expand(wsize, false);
    std::copy(d.Buffer(), d.Buffer()+wsize, Buffer());
    std::memset(Buffer()+wsize, 0, BufferSize()-wsize);
    SetReadMode();
    return *this;
}

KeyData::KeyData(size_t n, const void* p): TMessage(kMESS_BINARY, sizeof(UInt_t) + n) {
    whut();
    WriteUInt(n);
    assert(p);
    memcpy(fBufCur, p, n);
    fBufCur += n;
    wsize = fBufCur - Buffer();
    std::memset(fBufCur, 0, fBufMax-fBufCur);
    SetReadMode();
}

void KeyData::accumulate(const KeyData& kd) {
    auto w = What();
    if(w != kd.What()) throw std::domain_error("Incompatible accumulation types!");
    if(w == kMESS_INTS) accumulateV<int>(kd);
    else if(w == kMESS_DOUBLES) accumulateV<double>(kd);
    else throw std::domain_error("Non-accumulable type!");
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
void BinaryWriter::send<KeyData>(const KeyData& M) {
    start_wtx();
    send<UInt_t>(M.wSize());
    append_write(M.Buffer(), M.wSize());
    end_wtx();
}

template<>
KeyData* BinaryReader::receive<KeyData*>() {
    UInt_t s = receive<UInt_t>();
    auto buf = new char[s];
    _receive((void*)buf, s);
    return new KeyData(buf, s);
}

template<>
void BinaryReader::receive(KeyData& d) {
    UInt_t s = receive<UInt_t>();
    auto buf = new char[s];
    _receive((void*)buf, s);
    d = KeyData(buf, s);
}
