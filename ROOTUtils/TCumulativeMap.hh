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
#include <iostream>

/// TCumulative wrapper for std::map<key, value>
template<typename K_, typename V_>
class TCumulativeMap: public TCumulative, public map<K_,V_> {
public:
    typedef map<K_,V_> map_t;

    /// Constructor
    TCumulativeMap(const TString& nme = "", const TString& ttl = ""): TCumulative(nme,ttl) { }

    /// Scale contents by factor
    void Scale(Double_t s) override { for(auto& kv: *this) kv.second *= s; }
    /// Add another object of the same type
    void Add(const CumulativeData& CD, Double_t s = 1.) override {
        for(auto& kv: dynamic_cast<const TCumulativeMap<K_,V_>&>(CD)) Insert(kv.first, kv.second*s);
    }
    /// debugging contents print
    void display() const override {
        std::cout << "TCumulativeMap '" << name << "'\n";
        //for(auto& kv: *this) std::cout << "\t" << kv.first << " -> " << kv.second << std::endl;
    }
    /// clear data contents
    void ClearCumulative() override { this->clear(); }
    /// Insert/add contents
    void Insert(const K_& k, const V_& v) { (*this)[k] += v; }

    /// Get sum total of all contents
    V_ GetTotal() const { V_ sm = {}; for(auto& kv: *this) sm += kv.second; return sm; }

    ClassDefOverride(TCumulativeMap, 3)
};

#endif
