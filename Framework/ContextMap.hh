/// \file ContextMap.hh Utility for (context-scoped) cascading variables lookup
// -- Michael P. Mendenhall, LLNL 2019

#ifndef CONTEXTMAP_HH
#define CONTEXTMAP_HH

#include <utility>
#include <map>
#include <vector>
#include <stdexcept>
#include <typeinfo>
#include "NoCopy.hh"

/// Utility for (context-scoped) cascading variables lookup
class ContextMap: private NoCopy {
public:
    /// Default constructor
    explicit ContextMap(ContextMap* _parent = nullptr): parent(_parent) { }
    /// Destructor
    ~ContextMap() { for(auto& kv: dat) if(kv.second.second) kv.second.second->deletep(kv.second.first); }

    /// get active context (create if none previously exist)
    static ContextMap& getContext();
    /// push new active context
    static ContextMap& pushContext();
    /// delete active context (invalidates references); return whether any were deleted
    static bool popContext();
    /// active context stack
    static std::vector<ContextMap*>& getContextStack();

    /// clear value
    template<typename U, typename T = void>
    static void unset() { getContext()._unset<U,T>(); }

    /// clear value
    template<typename U, typename T = void>
    void _unset() { disown(tp_id<U,T>()); }

    /// set labeled object
    template<typename U, typename T = void>
    static void setPtr(U* x) { getContext()._setPtr<U,T>(x); }

    /// set labeled object
    template<typename U, typename T = void>
    void _setPtr(U* x) {
        _unset<U,T>();
        dat[tp_id<U,T>()] = {x, nullptr};
    }

    /// set labeled object with copy
    template<typename U, typename T = void>
    static void setCopy(const U& x) { getContext()._setCopy<U,T>(x); }

    /// set labeled object with copy
    template<typename U, typename T = void>
    void _setCopy(const U& x) {
        _unset<U,T>();
        dat[tp_id<U,T>()] = {new U(x), new owner_t<U>};
    }

    /// get (possibly-nullptr) U* labeled by T
    template<typename U, typename T = void>
    static U* get() { return getContext()._get<U,T>(); }

    /// get (possibly-nullptr) U* labeled by T
    template<typename U, typename T = void>
    U* _get() {
        auto it = dat.find(tp_id<U,T>());
        if(it != dat.end()) return static_cast<U*>(it->second.first);
        return parent? parent->_get<U,T>() : nullptr;
    }

    /// get reference labeled by T; throw if nonexistent
    template<typename U, typename T = void>
    U& _rget() {
        auto o = _get<U,T>();
        if(!o) throw std::runtime_error("Requested context object not set");
        return *o;
    }

    /// get or construct with default arguments
    template<typename U, typename T = void, typename... Args>
    static U& getDefault(Args&&... a) { return getContext()._getDefault<U,T,Args...>(std::forward<Args>(a)...); }

    /// get or construct with default arguments
    template<typename U, typename T = void, typename... Args>
    U& _getDefault(Args&&... a) {
        auto o = _get<U,T>();
        if(o) return *o;
        o = new U(std::forward<Args>(a)...);
        _setPtr<U,T>(o);
        return *o;
    }

    /// assign value if present
    template<typename U, typename T = void>
    static void lookup(U& x) { getContext()._lookup<U,T>(x); }

    /// assign value if present
    template<typename U, typename T = void>
    void _lookup(U& x) {
        auto o = _get<U,T>();
        if(o) x = *o;
    }

protected:

    /// type hashes pair
    typedef std::pair<size_t, size_t> tp_t;
    /// create type pair identifier
    template<typename T, typename U>
    static constexpr tp_t tp_id() { return {typeid(T).hash_code(), typeid(U).hash_code()}; }

    /// Base pointer ownership wrapper
    class _owner_t {
    public:
        /// delete pointer
        virtual void deletep(void* p) const = 0;
        /// clone pointer
        virtual void* clone(const void* p) const = 0;
        /// clone this class
        virtual _owner_t* clowner() const = 0;
    };

    /// Deleter for a class
    template<typename T>
    class owner_t: public _owner_t {
    public:
        /// delete pointer
        void deletep(void* p) const override { delete static_cast<T*>(p); }
        /// clone pointer
        void* clone(const void* p) const override { return p? new T(*static_cast<T*>(p)) : nullptr; }
        /// clone this class
        _owner_t* clowner() const override { return new owner_t; }
    };

    std::map<tp_t, std::pair<void*, _owner_t*>> dat; ///< data map
    ContextMap* parent = nullptr;                    ///< parent reference

    /// remove previous contents
    void disown(tp_t x);
};

/// Context-settable singleton helper: adds singleton get/set/lookup to class
// use "class Foo: public s_context_singleton<Foo>"
template<typename T>
struct s_context_singleton {
    /// set context singleton from this object
    void set() { ContextMap::setCopy<T,T>(*static_cast<const T*>(this)); }

    /// get context singleton, with default settings
    template<typename... Args>
    static T& get(Args&&... a) { return ContextMap::getDefault<T,T,Args...>(std::forward<Args>(a)...); }

    /// update this object from context singleton
    void lookup() { ContextMap::lookup<T,T>(*static_cast<const T*>(this)); }
};

/// Context-settable singleton helper; adds instance() static function to class, pointing to current singleton object
// use "class Foo: public s_context_singleton_ptr<Foo>"
template<typename T>
struct s_context_singleton_ptr {
    /// Get current singleton instance
    static T* instance() { return ContextMap::get<T,s_context_singleton_ptr>(); }
protected:
    /// Register on construction
    s_context_singleton_ptr() {
        if(ContextMap::get<T,s_context_singleton_ptr>() != nullptr) throw std::runtime_error("Duplicate singleton instantiation");
        ContextMap::setPtr<T,s_context_singleton_ptr>(static_cast<T*>(this));
    }
    /// Deregister on destruction
    virtual ~s_context_singleton_ptr() { ContextMap::unset<T,s_context_singleton_ptr>(); }
};

#endif
