/// \file PointSelector.cc

#include "PointSelector.hh"
#include "DiskBIO.hh"

void PointSelector::display() const {
    printf("PointSelector for %zu dimensions in %zu partitions:\n", Ntot, parts.size());
    for(auto& p: parts) printf("\t%zu dimensions: %zu x %zu points (at %zu)\n", p.N, p.npts, p.Nsub, p.QRNGn);
}

void PointSelector::addPart(size_t N, size_t npts) {
    if(!N || !npts) return;
    for(auto& p: parts) p.Nsub *= npts;
    parts.emplace_back(N,npts);
    v0.resize(v0.size() + N);
    Ntot += N;
    if(Ntot != v0.size()) throw;
}

void PointSelector::skipTo(size_t i) {
    for(auto it = parts.rbegin(); it != parts.rend(); ++it) {
        auto j = i / it->Nsub;
        //assert(j <= it->QRNGn);
        it->QRNG.Skip(j - it->QRNGn);
    }
}

PointSelector::vec_t PointSelector::next() {
    size_t i = v0.size();
    subgroup = parts.size();
    for(auto it = parts.rbegin(); it != parts.rend(); ++it) {
        i -= it->N;
        it->QRNG.Next(v0.data()+i);
        --subgroup;
        if(it->QRNGn++ % it->npts) break;
    }
    return v0;
}

template<>
void BinaryWriter::send<PointSelector::axpart>(const PointSelector::axpart& p) {
    send(p.N);
    send(p.npts);
    send(p.Nsub);
    send(p.QRNGn);
}

template<>
void BinaryReader::receive<PointSelector::axpart>(PointSelector::axpart& p) {
    receive(p.N);
    receive(p.npts);
    receive(p.Nsub);
    receive(p.QRNGn);
    p.QRNG = ROOT::Math::QuasiRandomNiederreiter(p.N);
    p.QRNG.Skip(p.QRNGn);
}

std::ostream& operator<<(std::ostream& o, const PointSelector& p) {
    IOStreamBWrite b(o);
    b.start_wtx();
    b.send(p.subgroup);
    b.send(p.parts);
    b.send(p.Ntot);
    b.send(p.v0);
    b.end_wtx();

    return o;
}

std::istream& operator>> (std::istream &i, PointSelector& p) {
    IOStreamBRead b(i);
    b.receive(p.subgroup);
    b.receive(p.parts);
    b.receive(p.Ntot);
    b.receive(p.v0);
    return i;
}
