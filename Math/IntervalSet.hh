/// \file IntervalSet.hh Collection of disjoint intervals
// -- Michael P. Mendenhall, 2018

#ifndef INTERVALSET_HH
#define INTERVALSET_HH

#include "Interval.hh"
#include <set>
using std::set;
#include <stdlib.h> // for size_t
#include <numeric>  // for std::accumulate

/// Collection of disjoint intervals
template<typename T = double>
class IntervalSet: protected set<Interval<T>> {
public:
    /// interval specified by (start, end)
    typedef Interval<T> interval_t;
    /// parent type
    typedef set<interval_t> super;
    // expose useful functions
    using super::size;
    using super::begin;
    using super::rbegin;
    using super::insert;
    using super::end;
    using super::erase;
    using super::lower_bound;
    using super::upper_bound;

    /// add interval to set, merging with any overlapping intervals
    void operator+=(interval_t i) {
        if(i.isNull()) return; // do not keep null intervals
        ++nIndividual;
        tIndividual += i.dl();

        // completely outside of existing range?
        if(!size() || i.hi < begin()->lo || i.lo > rbegin()->hi) { insert(i); return; }

        // find first interval after or overlapping start of i
        auto it0 = lower_bound(i);
        if(it0 != begin() && std::prev(it0)->hi >= i.lo) it0--;
        if(it0 != end() && i.hi >= it0->lo) i.lo = std::min(i.lo, it0->lo);

        // find first interval after end of i
        auto it1 = upper_bound({i.hi, i.hi});
        i.hi = std::max(i.hi, std::prev(it1)->hi);
        if(it1 != begin() && std::prev(it1)->hi >= i.lo) i.hi = std::max(i.hi, std::prev(it1)->hi);

        erase(it0, it1);
        insert(i);

        // summarize old intervals
        if(dtMax) summarize(i.lo - dtMax);
    }

    /// combine (OR) with other intervals
    void operator+=(const IntervalSet& rhs) {
        for(auto i: rhs) *this += i;
        nIndividual += rhs.nIndividual;
        tIndividual += rhs.tIndividual;
    }

    /// intersection (AND) with other intervals
    void operator&=(const IntervalSet& rhs) {
        IntervalSet inew;
        auto it0 = begin();
        auto it1 = rhs.begin();
        while(it0 != end() && it1 != rhs.end()) {
            if(it0->second < it1->first) ++it0;
            else if(it1->second < it0->first) ++it1;
            else {
                inew += {std::max(it0->first, it1->first), std::min(it0->second, it1->second)};
                if(it0->second < it1->second) ++it0;
                else ++it1;
            }
        }
        *this = inew;
        nIndividual += rhs.nIndividual;
        tIndividual += rhs.tIndividual;
    }

    /// collapse all intervals ending before specified value into summary
    void summarize(T t0) {
        auto it = lower_bound({t0,t0});
        for(auto i = begin(); i != it; ++i) {
            ++nSummary;
            tSummary += i->dl();
        }
        erase(begin(), it);
    }
    /// flush all active intervals into summary
    void flush() {
        nSummary += size();
        for(auto& i: *this)  tSummary += i.dl();
        super::clear();
    }

    size_t nSummary = 0;    ///< number of summarized non-tracked intervals
    size_t dtMax = 0;       ///< maximum length to store (0 to disable)
    T tSummary = {};        ///< content of summarized non-tracked intervals
    T tIndividual = {};     ///< span of individual intervals added before overlap
    size_t nIndividual = 0; ///< number of individual intervals before merge

    /// number of intervals
    size_t n() const { return size() + nSummary; }

    /// total of all intervals
    T total() const {
        return std::accumulate(begin(), end(), tSummary,
                               [](T a, interval_t b) { return a + b.dl(); });
    }
};

#endif
