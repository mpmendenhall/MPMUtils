/// \file DynamicHistogram.cc
#include "DynamicHistogram.hh"
using std::pair;

void DHBinData::operator+=(const DHBinData& r) {
    wx += r.wx;
    wxx += r.wxx;
    w += r.w;
}

void DynamicHistogram::fill(double x, double w) {
    DHBinData d(x,w);
    total += d;

    if(!dat.size()) {
        dat.emplace(bincenter(d),d);
        return;
    }

    auto it = choosebin(d);
    if(it == dat.end()) {
        dat.emplace(bincenter(d),d);
    } else {
        it->second += d;
        double c = bincenter(it->second);
        if(c != it->first) {
            d = it->second;
            dat.erase(it);
            dat.emplace(c,d);
        }
    }
}

DHBinData DynamicHistogram::getMax() const {
    DHBinData d;
    for(auto const& kv: dat) if(d.w < kv.second.w) d = kv.second;
    return d;
}
