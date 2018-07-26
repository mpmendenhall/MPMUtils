/// \file IntervalSet.hh Collection of disjoint intervals
// Michael P. Mendenhall, 2018

#ifndef INTERVALSET_HH
#define INTERVALSET_HH

#include <set>
using std::set;

/// Collection of disjoint intervals
template<typename T>
class IntervalSet {
public:
    /// interval specified by (start, end)
    typedef std::pair<T,T> interval;

    /// add interval to set, merging with any overlapping intervals
    void add(interval i) {
        if(i.second < i.first) std::swap(i.first, i.second); // assure end >= start
        // completely outside of existing range?
        if(!s.size() || i.second < s.begin()->first || i.first > s.rbegin()->second) { s.insert(i); return; }

        // find first interval after or overlapping start of i
        auto it0 = s.lower_bound(i);
        if(it0 != s.begin() && std::prev(it0)->second >= i.first) it0--;
        if(it0 != s.end() && i.second >= it0->first) i.first = std::min(i.first, it0->first);

        // find first interval after end of i
        auto it1 = s.upper_bound({i.second, i.second});
        i.second = std::max(i.second, std::prev(it1)->second);
        if(it1 != s.begin() && std::prev(it1)->second >= i.first) i.second = std::max(i.second, std::prev(it1)->second);

        s.erase(it0, it1);
        s.insert(i);
    }

    /// collapse all intervals ending before specified value into summary
    void summarize(T t0) {
        auto it = s.lower_bound({t0,t0});
        if(it != s.end() && std::prev(it)->second > t0) it--;
        for(auto i = s.begin(); i != it; i++) {
            nSummary++;
            tSummary += i->second - i->first;
        }
        s.erase(s.begin(), it);
    }

    size_t nSummary = 0;    /// number of summarized non-tracked intervals
    T tSummary = 0;         /// constent of summarized non-tracked intervals

    /// number of intervals
    size_t n() const { return s.size() + nSummary; }

    /// total of all intervals
    T total() const {
        T t = tSummary;
        for(auto i: s) t += i.second - i.first;
        return t;
    }

//protected:
    /// interval sorting by start point
    class icomp {
    public:
        bool operator()(const interval& a, const interval& b) const { return a.first < b.first; }
    };

    set<interval, icomp> s;    ///< the intervals
};

#endif
