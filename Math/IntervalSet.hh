/// \file IntervalSet.hh Collection of disjoint intervals
// Michael P. Mendenhall, 2018

#ifndef INTERVALSET_HH
#define INTERVALSET_HH

#include <set>
using std::set;
#include <stdlib.h> // for size_t

    /// interval sorting by start point
template<typename T>
class CompareFirst {
public:
    bool operator()(const T& a, const T& b) const { return a.first < b.first; }
};

/// Collection of disjoint intervals
template<typename T>
class IntervalSet: public set<std::pair<T,T>, CompareFirst<std::pair<T,T>>> {
public:
    /// interval specified by (start, end)
    typedef std::pair<T,T> interval;

    /// add interval to set, merging with any overlapping intervals
    void operator+=(interval i) {
        if(i.second < i.first) std::swap(i.first, i.second); // assure end >= start
        // completely outside of existing range?
        if(!this->size() || i.second < this->begin()->first || i.first > this->rbegin()->second) { this->insert(i); return; }

        // find first interval after or overlapping start of i
        auto it0 = this->lower_bound(i);
        if(it0 != this->begin() && std::prev(it0)->second >= i.first) it0--;
        if(it0 != this->end() && i.second >= it0->first) i.first = std::min(i.first, it0->first);

        // find first interval after end of i
        auto it1 = this->upper_bound({i.second, i.second});
        i.second = std::max(i.second, std::prev(it1)->second);
        if(it1 != this->begin() && std::prev(it1)->second >= i.first) i.second = std::max(i.second, std::prev(it1)->second);

        this->erase(it0, it1);
        this->insert(i);
    }

    /// combine (OR) with other intervals
    void operator+=(const IntervalSet& rhs) { for(auto i: rhs) *this += i; }

    /// intersection (AND) with other intervals
    void operator&=(const IntervalSet& rhs) {
        IntervalSet inew;
        auto it0 = this->begin();
        auto it1 = rhs.begin();
        while(it0 != this->end() && it1 != rhs.end()) {
            if(it0->second < it1->first) ++it0;
            else if(it1->second < it0->first) ++it1;
            else {
                inew += {std::max(it0->first, it1->first), std::min(it0->second, it1->second)};
                if(it0->second < it1->second) ++it0;
                else ++it1;
            }
        }
        *this = inew;
    }

    /// collapse all intervals ending before specified value into summary
    void summarize(T t0) {
        auto it = this->lower_bound({t0,t0});
        if(it != this->end() && std::prev(it)->second > t0) it--;
        for(auto i = this->begin(); i != it; i++) {
            nSummary++;
            tSummary += i->second - i->first;
        }
        this->erase(this->begin(), it);
    }

    size_t nSummary = 0;    /// number of summarized non-tracked intervals
    T tSummary = 0;         /// constent of summarized non-tracked intervals

    /// number of intervals
    size_t n() const { return this->size() + nSummary; }

    /// total of all intervals
    T total() const {
        T t = tSummary;
        for(auto i: *this) t += i.second - i.first;
        return t;
    }

};

#endif
