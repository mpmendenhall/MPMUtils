/// \file TCumulativeMap.hh TCumulative wrapper for std::map<key, value>
// This file was produced under the employ of the United States Government,
// and is consequently in the PUBLIC DOMAIN, free from all provisions of
// US Copyright Law (per USC Title 17, Section 105).
// 
// -- Michael P. Mendenhall, 2016

#ifndef TCUMULATIVEMAP_HH
#define TCUMULATIVEMAP_HH

#include "TCumulative.hh"
#include <map>
using std::map;

/// TCumulative wrapper for std::map<key, value>
template<typename K_, typename V_>
class TCumulativeMap: public TCumulative {
public:
    /// Constructor
    TCumulativeMap(const TString& nme = "", const TString& ttl = ""): TCumulative(nme,ttl) { }
    
    /// Scale contents by factor
    virtual void _Scale(Double_t s) { for(auto& kv: fDat) kv.second *= s; }
    /// Add another object of the same type
    virtual void _Add(const CumulativeData* CD, Double_t s = 1.) {
        auto m2 = dynamic_cast<const TCumulativeMap<K_,V_>*>(CD);
        if(!m2) return;
        for(auto& kv: m2->fDat) Insert(kv.first, kv.second*s);
    }
    /// clear data contents
    virtual void _Clear() { fDat.clear(); }
    
    /// Add data contents
    void Insert(const K_& k, V_ v) {
        auto it = fDat.find(k);
        if(it == fDat.end()) fDat.emplace(k, v);
        else it->second += v;
    }
    /// Get number of keys
    size_t Size() const { return fDat.size(); }
    /// get data entry
    V_& operator[](const K_& k) { return fDat[k]; }
    /// Get sum total of all contents
    V_ GetTotal() const { V_ sm = V_(); for(auto& kv: fDat) sm += kv.second; return sm; }
    /// Get internal data
    map<K_,V_>& GetData() { return fDat; }
    /// Get internal data, const
    const map<K_,V_>& GetData() const { return fDat; }
    
protected:
    map<K_,V_> fDat;    ///< data contents
    ClassDef(TCumulativeMap, 1)
};

#endif
