/// @file DataSource.hh Base class for providing a stream of objects
// -- Michael P. Mendenhall, LLNL 2022

#ifndef DATASOURCE_HH
#define DATASOURCE_HH

#include <limits>
#include <vector>
using std::vector;

/// Reader from a file
class _FileSource {
public:
    virtual void openInput(const string& f) { infile_name = f; }
    string infile_name;     ///< input filename
};

/// Type-independent DataSource base class
class _DataSource {
public:
    /// maximum "infinite" entries
    static constexpr size_t max_entries = std::numeric_limits<size_t>::max();

    /// Polymorphic destructor
    virtual ~_DataSource() { }

    /// Initialize for read start
    virtual void init_dsource() { }
    /// Reset to start
    virtual void reset() { nread = 0; id_current_evt = -1; }
    /// Skip ahead n items
    virtual bool skip(size_t n) = 0;

    /// Estimate remaining data size (no loop)
    virtual size_t entries() const { return 0; }
    /// Remaining entries, including load limit
    size_t entries_remaining() const { auto n0 = entries(); return nLoad >= 0 && size_t(nLoad) < n0? nLoad - nread : n0 - nread; }
    /// Estimate remaining data size (including loop)
    size_t entries_optloop() { return doLoop? max_entries : entries_remaining(); }
    /// get number of rows already read
    size_t getNRead() const { return nread; }

    bool doLoop = false;    /// whether to do infinite looping
    int nLoad = -1;         ///< entries loading limit; set >= 0 to apply

protected:
    size_t nread = 0;               ///< number of items already read
    int64_t id_current_evt = -1;    ///< event identifier of next_read
};

/// Virtual base class for loading a stream of objects
template<class C>
class DataSource: public _DataSource {
public:
    /// retrieved value type
    typedef typename std::remove_const<C>::type val_t;

    /// Fill supplied item with next object; return whether item has been updated
    virtual bool next(val_t&) = 0;
    /// Skip ahead n items
    bool skip(size_t n) override { val_t x; while(n--) if(!next(x)) return false; return true; }
    /// pop with infinite looping
    bool next_loop(val_t& o) { if(next(o)) return true; reset(); return next(o); }
    /// next with optional looping
    bool next_optloop(val_t& o) { return doLoop? next_loop(o) : next(o); }
    /// get identifying number for value type
    static int64_t getIdentifier(const val_t& i) { return i.evt; }
};

/// Sequence of DS ~ DataSource
template<class DS>
class DataSourceSeq: public DS {
public:
    /// Underlying datasource type
    typedef DS dsrc_t;
    /// Underlying data type
    typedef typename dsrc_t::val_t val_t;

    /// Constructor inheritance
    using dsrc_t::dsrc_t;

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
    size_t entries() const override {
        size_t e = 0;
        for(auto j = i; j < v.size(); ++j) {
            auto ee = v[j]->entries();
            if(ee == dsrc_t::max_entries) return dsrc_t::max_entries;
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
