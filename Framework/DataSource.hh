/// \file DataSource.hh Base class for providing a stream of objects
// Michael P. Mendenhall, LLNL 2019

#ifndef DATASOURCE_HH
#define DATASOURCE_HH

#include <vector>
#include <functional> // for std::reference_wrapper

/// Virtual base class for accepting a stream of objects
template<class C>
class DataSource {
public:
    /// retrieved value type
    typedef C val_t;

    /// Fill supplied item with next object; return whether item has been updated
    virtual bool next(val_t&) = 0;
    /// pop with infinite looping
    bool next_loop(val_t& o) { if(pop(o)) return true; reset(); return next(o); }
    /// Reset to start
    virtual void reset() { }
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
    virtual void addStream(dsrc_t& S) { v.push_back(S); }

    /// Fill o with next object; return whether o updated
    bool next(val_t& o) override {
        while(i < v.size() && !v[i].get().next(o)) { nextSource(); ++i; }
        return i < v.size();
    }

    /// Reset to start
    void reset() override { while(i) v[--i].get().reset(); }

protected:
    /// Called when switching to next source
    virtual void nextSource() { }

    std::vector<std::reference_wrapper<dsrc_t>> v;    ///< underlying sources
    size_t i = 0;   ///< current position in sources list
};

#endif
