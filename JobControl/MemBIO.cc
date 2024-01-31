/// @file

#include "MemBIO.hh"

void MemBReader::ignore(size_t n) {
    if(pR + n > eR) throw std::runtime_error("Invalid ignore quantity");
    pR += n;
}

void MemBReader::read(void* vptr, size_t size) {
    if(pR + size > eR) throw std::runtime_error("Invalid receive allocation");
    std::memcpy(vptr,pR,size);
    pR += size;
}


//--------------------------------------------


void MemBWriter::_send(const void* vptr, size_t size) {
    if(pW + size > eW) throw -1;
    std::memcpy(pW,vptr,size);
    pW += size;
}


//--------------------------------------------


void DequeBIO::read(void* vptr, size_t s) {
    if(size() < s) throw std::domain_error("Insufficient buffered data!");
    auto v = reinterpret_cast<char*>(vptr);
    while(s--) { *(v++) = front(); pop_front(); }
}


//--------------------------------------------


void BufferingReader::read(void* vptr, size_t size) {
    size_t rsize = rpos + size;
    if(rsize > dat.size()) {
        load_buf_upto(dchunk + rsize - dat.size());
        rsize = rpos + size;
        if(rsize > dat.size()) load_buf(rsize - dat.size());
    }
    std::memcpy(vptr, dat.data() + rpos, size);
    rpos += size;
}

size_t BufferingReader::read_upto(void* vptr, size_t size) {
    size_t rmax = std::min(rpos + size, dat.size());
    std::memcpy(vptr, dat.data()+rpos, rmax);
    rpos += rmax;
    if(rpos > dchunk/2) load_buf_upto(dchunk);
    return rmax;
}

void BufferingReader::rebuffer() {
    dat.erase(dat.begin(), dat.begin() + rpos);
    rpos = 0;
}

void BufferingReader::load_buf(size_t s) {
    if(rpos + s + dchunk/2 > dat.size()) rebuffer();
    auto s0 = dat.size();
    dat.resize(s0 + s);
    R.read(&dat[s0], s);
}

void BufferingReader::load_buf_upto(size_t s) {
    if(rpos + s + dchunk/2 > dat.size()) rebuffer();
    auto s0 = dat.size();
    dat.resize(s0 + s);
    s0 += R.read_upto(&dat[s0], s);
    dat.resize(s0);
}
