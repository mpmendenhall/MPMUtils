/// @file KeyTable.cc

#include "KeyTable.hh"
#include <cstdint>

KeyData::KeyData(int what, size_t n): TMessage(what, n) {
    wsize = 2*sizeof(UInt_t)+n;
    std::memset(Buffer()+2*sizeof(UInt_t), 0, BufferSize()-2*sizeof(UInt_t));
    SetReadMode();
}

KeyData& KeyData::operator=(const KeyData& d) {
    if(&d == this) return *this;  // self-copy???

    wsize = d.wSize();
    Expand(wsize, false);
    std::copy(d.Buffer(), d.Buffer()+wsize, Buffer());
    std::memset(Buffer()+wsize, 0, BufferSize()-wsize);
    SetWhat(d.What());
    SetReadMode();
    return *this;
}

KeyData& KeyData::operator+=(const KeyData& kd) {
    auto w = What();
    if(w != kd.What()) throw std::domain_error("Incompatible accumulation types!");

    if(w < kMESS_BINARY) throw std::domain_error("Non-accumulable type!");

    if(w < kMESS_ARRAY) w -= kMESS_BINARY;
    else w -= kMESS_ARRAY;

    /* */if(w == typeID<  int8_t>()) accumulate<int8_t>(kd);
    else if(w == typeID< int16_t>()) accumulate<int16_t>(kd);
    else if(w == typeID< int32_t>()) accumulate<int32_t>(kd);
    else if(w == typeID< int64_t>()) accumulate<int64_t>(kd);
    else if(w == typeID< uint8_t>()) accumulate<uint8_t>(kd);
    else if(w == typeID<uint16_t>()) accumulate<uint16_t>(kd);
    else if(w == typeID<uint32_t>()) accumulate<uint32_t>(kd);
    else if(w == typeID<uint64_t>()) accumulate<uint64_t>(kd);
    else if(w == typeID<   float>()) accumulate<float>(kd);
    else if(w == typeID<  double>()) accumulate<double>(kd);
    else if(w == typeID<long double>()) accumulate<long double>(kd);
    else throw std::domain_error("Non-accumulable type!");

    return *this;
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

KeyTable& KeyTable::operator=(const KeyTable& k) {
    if(&k == this) return *this;
    Clear();
    (map<string, KeyData*>&)(*this) = k;
    for(auto& kv: *this) kv.second = new KeyData(*kv.second);
    return *this;
}

bool KeyTable::_Set(const string& s, KeyData* v) {
    assert(v);

    auto it = find(s);
    if(it != end()) {
        delete it->second;
        it->second = v;
        return true;
    }

    (*this)[s] = v;
    return false;
}

/////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////

template<>
void BinaryWriter::send<KeyData>(const KeyData& M) {
    start_wtx();
    auto ds = M.wSize()-2*sizeof(UInt_t);
    send<UInt_t>(ds);
    send<UInt_t>(M.What());
    append_write(M.data(), ds);
    end_wtx();
}

template<>
KeyData* BinaryReader::receive<KeyData*>() {
    UInt_t s = receive<UInt_t>();
    UInt_t w = receive<UInt_t>();
    auto d = new KeyData(w, s);
    read(static_cast<void*>(d->data()), s);
    return d;
}

template<>
void BinaryReader::receive(KeyData& d) {
    UInt_t s = receive<UInt_t>();
    Int_t w = receive<UInt_t>();
    d = KeyData(w,s);
    read(static_cast<void*>(d.data()), s);
}
