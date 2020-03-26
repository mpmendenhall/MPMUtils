/// \file DataFrame.hh Manage lifecycle of generic data objects
// -- Michael P. Mendenhall, 2018

#ifndef DATAFRAME
#define DATAFRAME

#include "AllocPool.hh"
#include <map>
using std::map;
#include <string>
using std::string;
#include <typeinfo>
#include <typeindex>
#include <stdlib.h>
#include <stdio.h>
#include <boost/core/noncopyable.hpp>
#include <mutex>
#include <cassert>

class DataFrame;

/// Enumerate classes contained in DataFrames, and manage their lifecycles.
class DataManager: private boost::noncopyable, protected LockedAllocPool<DataFrame> {
public:
    friend class DataFrame;

    /// Destructor
    ~DataManager();

    /// Get empty data frame (thread-safe) with reference count 1
    DataFrame& getFrame();

    /// Base for handling void* data frame contents
    class TypeManagerBase: private boost::noncopyable {
    public:
        /// Constructor with index
        TypeManagerBase(size_t i): idx(i) { }
        /// Destructor
        virtual ~TypeManagerBase() { }
        /// Type information
        virtual const std::type_info& getType() const = 0;
        /// Create/get one of managed type
        virtual void* create() = 0;
        /// Dispose of managed type
        virtual void dispose(void* p) = 0;
        size_t idx; ///< enumeration index
    };

    /// Manager for a particular class of data
    template<class T>
    class TypeManager: public TypeManagerBase, protected LockedAllocPool<T> {
    public:
        using TypeManagerBase::TypeManagerBase;
        /// Type information
        const std::type_info& getType() const override { return typeid(T); }
        /// Create managed type
        void* create() override { return this->get(); }
        /// Dispose of managed type
        void dispose(void* p) override { this->put((T*)p); }
    };

    /// Assign/get manager for a new type
    template<class T>
    TypeManagerBase& getType() {
        std::unique_lock<std::mutex> lk(typesLock);
        auto tp = std::type_index(typeid(T));
        auto it = types.find(tp);
        if(it != types.end()) return *it->second;
        auto tm = types.emplace(tp, new TypeManager<T>(types.size())).first->second;
        dtypes.push_back(tm);
        return *tm;
    }

    /// Display datatype contents
    virtual void display() const {
        printf("DatastreamManager for frame containing:\n");
        for(auto t: dtypes) printf("\t> %s\n", t->getType().name());
    }

protected:
    map<std::type_index, TypeManagerBase*> types;   ///< managed data types
    std::mutex typesLock;                           ///< lock on types
    vector<TypeManagerBase*> dtypes;                ///< managed data types in index order
};

/// Opaque collection of items, organized by a DataManager
class DataFrame: protected vector<void*>, private boost::noncopyable {
public:
    friend class DataManager;

    /// Destructor
    ~DataFrame() { assert(!nrefs); }

    /// get data of specified class; nullptr if undefined
    template<class T>
    T*& get(bool create = false) {
        auto& mp = M->getType<T>();
        if(mp.idx >= size()) resize(mp.idx+1, nullptr);
        if(create && !(*this)[mp.idx]) (*this)[mp.idx] = mp.create();
        return (T*&)(*this)[mp.idx];
    }

    /// get access (creating if necessary) to specified class in frame
    template<class T>
    T& access() { return *get<T>(true); }

    /// for re-use pool
    using vector<void*>::clear;
    /// increment reference counter
    void claim() { nrefs++; }
    /// decrement reference counter; dispose of contents and return if unused
    void release();
    /// get reference count
    int getRefs() const { return nrefs; }

    bool drop = false;  ///< flag to drop frame from processing
    double i;           ///< sort order index

protected:
    DataManager* M;     ///< Manager associated with frame
    int nrefs = 0;      ///< reference counter
};

class FrameSink;

/// Base class providing frames for analysis, and receiving back when completed.
class FrameSource {
public:
    /// receive frame back after processing completed
    virtual void finished(DataFrame& F, FrameSink&) { F.release(); }
};

/// Base class for processing stream of frames
class FrameSink {
public:
    /// start receiving a series of data frames
    virtual void start_data(DataFrame& F) { startFrame = &F; }
    /// process next data frame in series supplied by source
    virtual void receive(DataFrame&, FrameSource&) { }
    /// end series of data frames
    virtual void end_data(DataFrame&) { }

    /// indicate whether object retains frame, calling FrameSource::finished at a delayed time
    virtual bool keepsframe() const { return false; }
    /// indicate whether object may receive() on multiple simultaneous threads
    virtual bool threadsafe() const { return false; }

    string name;                        ///< debugging name

    /// show summary of time use
    virtual void displayTimeSummary(int d = 0) const;

    /// time use profiling
    struct profile_t {
        double t_start = 0;
        double t_receive = 0;
        double t_end = 0;
        void operator+=(const profile_t& p) { t_start += p.t_start; t_receive += p.t_receive; t_end += p.t_end; }
        void display() const { printf("%.2f\t%.2f\t%.2f", t_start, t_receive, t_end); }
    } timeUse;

protected:
    DataFrame* startFrame = nullptr;    ///< start-of-data (run globals) frame; always assume kept.
};

#endif
