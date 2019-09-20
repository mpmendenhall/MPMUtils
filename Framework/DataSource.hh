/// \file DataSource.hh Base class for providing a stream of objects
// Michael P. Mendenhall, LLNL 2019

#ifndef DATASOURCE_HH
#define DATASOURCE_HH

#include <vector>
using std::vector;

/// Virtual base class for accepting a stream of objects
template<class C>
class DataSource {
public:
    /// retrieved value type
    typedef C val_t;
    /// maximum "infinite" entries
    static constexpr size_t max_entries = std::numeric_limits<size_t>::max();

    /// Virtual destructor
    virtual ~DataSource() { }

    /// Fill supplied item with next object; return whether item has been updated
    virtual bool next(val_t&) = 0;
    /// Skip ahead n items
    virtual bool skip(size_t n) { val_t x; while(n--) if(!next(x)) return false; return true; }
    /// pop with infinite looping
    bool next_loop(val_t& o) { if(next(o)) return true; reset(); return next(o); }
    /// Reset to start
    virtual void reset() { }
    /// Estimate remaining data size (including loop)
    size_t entries_optloop() { return doLoop? max_entries  : entries(); }
    /// Estimate remaining data size (no loop)
    virtual size_t entries() { return 0; }

    /// whether to do infinite looping
    bool doLoop = false;
    /// next with optional looping
    bool next_optloop(val_t& o) { return doLoop? next_loop(o) : next(o); }
};

/// Sequence of D ~ DataSource
template<class D>
class DataSourceSeq: public D {
public:
    /// Underlying datasource type
    typedef D dsrc_t;
    /// Underlying data type
    typedef typename dsrc_t::val_t val_t;

    /// Constructor inheritance
    using D::D;

    /// Add stream
    virtual void addStream(dsrc_t& S) { v.push_back(&S); }

    /// Fill o with next object; return whether o updated
    bool next(val_t& o) override {
        while(i < v.size() && !v[i]->next(o)) { nextSource(); ++i; }
        return i < v.size();
    }

    /// Reset to start
    void reset() override { while(i) v[--i]->reset(); }

    /// Estimate remaining data size (no loop)
    size_t entries() override {
        size_t e = 0;
        for(auto j = i; j < v.size(); ++j) {
            auto ee = v[j]->entries();
            if(ee == D::max_entries) return D::max_entries;
            e += ee;
        }
        return e;
    }


protected:
    /// Called when switching to next source
    virtual void nextSource() { }

    vector<dsrc_t*> v;  ///< underlying sources
    size_t i = 0;       ///< current position in sources list
};

#endif
