/// \file IntervalSet.hh Collection of disjoint intervals
// Michael P. Mendenhall, 2018

#ifndef INTERVALSET_HH
#define INTERVALSET_HH

#include <set>
using std::set;
#include <stdlib.h> // for size_t
#include <numeric>  // for std::accumulate
#include <limits>

/// An interval
template<typename T = double>
class Interval: public std::pair<T,T> {
public:
    /// Default constructor
    Interval(T a = {}, T b = {}): std::pair<T,T>(a,b) { }

    /// intersection of intervals
    const Interval& operator&=(Interval b) {
        if(this->second <= b.first || this->first >= b.second) *this = {};
        else *this = {std::max(this->first, b.first), std::min(this->second, b.second)};
        return *this;
    }
    /// intersection of intervals
    Interval operator&(Interval b) const { return b &= *this; }
    /// interval length
    operator T() const { return this->second - this->first; }
};

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
        if(i.second < i.first) std::swap(i.first, i.second); // assure end >= start
        ++nIndividual;
        tIndividual += i.second - i.first;

        // completely outside of existing range?
        if(!size() || i.second < begin()->first || i.first > rbegin()->second) { insert(i); return; }

        // find first interval after or overlapping start of i
        auto it0 = lower_bound(i);
        if(it0 != begin() && std::prev(it0)->second >= i.first) it0--;
        if(it0 != end() && i.second >= it0->first) i.first = std::min(i.first, it0->first);

        // find first interval after end of i
        auto it1 = upper_bound({i.second, i.second});
        i.second = std::max(i.second, std::prev(it1)->second);
        if(it1 != begin() && std::prev(it1)->second >= i.first) i.second = std::max(i.second, std::prev(it1)->second);

        erase(it0, it1);
        insert(i);

        // summarize old intervals
        if(dtMax) summarize(i.first - dtMax);
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
        if(it != end() && std::prev(it)->second > t0) it--;
        for(auto i = begin(); i != it; i++) {
            nSummary++;
            tSummary += i->second - i->first;
        }
        erase(begin(), it);
    }

    size_t nSummary = 0;    /// number of summarized non-tracked intervals
    size_t dtMax = 0;       ///< maximum length to store (0 to disable)
    T tSummary = 0;         /// content of summarized non-tracked intervals
    T tIndividual = 0;      /// span of individual intervals added before overlap
    size_t nIndividual = 0; /// number of individual intervals before merge

    /// number of intervals
    size_t n() const { return size() + nSummary; }

    /// total of all intervals
    T total() const {
        return std::accumulate(begin(), end(), tSummary,
                               [](T a, interval_t b) { return a + b.second - b.first; });
    }
};

#endif
