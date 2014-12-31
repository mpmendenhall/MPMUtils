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
        dat.insert(pair<double,DHBinData>(bincenter(d),d));
        return;
    }
    
    auto it = choosebin(d);
    if(it == dat.end()) {
        dat.insert(pair<double,DHBinData>(bincenter(d),d));
    } else {
        it->second += d;
        double c = bincenter(it->second);
        if(c != it->first) {
            d = it->second;
            dat.erase(it);
            dat.insert(pair<double,DHBinData>(c,d));
        }
    }
}

DHBinData DynamicHistogram::getMax() const {
    DHBinData d;
    for(auto it = dat.begin(); it != dat.end(); it++)
        if(d.w < it->second.w) d = it->second;
    return d;
}
