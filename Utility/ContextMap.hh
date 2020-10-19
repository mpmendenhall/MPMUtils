/// \file ContextMap.hh Utility for (context-scoped) cascading variables lookup
// -- Michael P. Mendenhall, LLNL 2019

#ifndef CONTEXTMAP_HH
#define CONTEXTMAP_HH

#include <utility>
#include <map>
#include <vector>
#include <string>
#include <stdexcept>
#include <typeinfo>

/// Utility for (context-scoped) cascading variables lookup
class ContextMap {
public:
    /// Default constructor
    explicit ContextMap(ContextMap* _parent = nullptr): parent(_parent) { }
    /// Copy constructor
    ContextMap(const ContextMap& M) { *this = M; }
    /// Copy Assignment
    ContextMap& operator=(const ContextMap& o);
    /// Destructor
    ~ContextMap() { for(auto& kv: dat) if(kv.second.second) kv.second.second->deletep(kv.second.first); }

    /// type hashes pair
    typedef std::pair<size_t, size_t> tp_t;
    /// create type pair identifier
    template<typename T, typename U>
    static constexpr tp_t tp_id() { return {typeid(T).hash_code(), typeid(U).hash_code()}; }

    /// get active context (create if none previously exist)
    static ContextMap& getContext();
    /// push new active context
    static ContextMap& pushContext();
    /// delete active context (invalidates references); return whether any were deleted
    static bool popContext();
    /// active context stack
    static std::vector<ContextMap*>& getContextStack();

    /// clear value
    template<typename T, typename U>
    void unset() { disown(tp_id<T,U>()); }

    /// set labeled object
    template<typename T, typename U>
    void setPtr(U* x) {
        unset<T,U>();
        dat[tp_id<T,U>()] = {x, nullptr};
    }

    /// set labeled object with copy
    template<typename T, typename U>
    void setCopy(const U& x) {
        unset<T,U>();
        dat[tp_id<T,U>()] = {new U(x), new owner_t<U>};
    }

    /// get (possibly-nullptr) U* labeled by T
    template<typename T, typename U>
    U* get() {
        auto it = dat.find(tp_id<T,U>());
        if(it != dat.end()) return (U*)it->second.first;
        return parent? parent->get<T,U>() : nullptr;
    }

    /// get reference labeled by T; throw if nonexistent
    template<typename T, typename U>
    U& rget() {
        auto o = get<T,U>();
        if(!o) throw std::runtime_error("Requested context object not set");
        return *o;
    }

    /// get or construct with default arguments
    template<typename T, typename U, typename... Args>
    U& getDefault(Args&&... a) {
        auto o = get<T,U>();
        if(o) return *o;
        o = new U(std::forward<Args>(a)...);
        setPtr<T,U>(o);
        return *o;
    }

    /// assign value if present
    template<typename T, typename U>
    void lookup(U& x) {
        auto o = get<T,U>();
        if(o) x = *o;
    }

protected:

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
        void deletep(void* p) const override { delete (T*)p; }
        /// clone pointer
        void* clone(const void* p) const override { return p? new T(*(T*)p) : nullptr; }
        /// clone this class
        _owner_t* clowner() const override { return new owner_t; }
    };

    std::map<tp_t, std::pair<void*, _owner_t*>> dat; ///< data map
    ContextMap* parent = nullptr;                    ///< parent reference

    /// remove previous contents
    void disown(tp_t x);
};

/// Context-settable singleton helper
template<typename T>
struct s_context_singleton {
    /// set context singleton
    void set() { ContextMap::getContext().setCopy<T,T>(*(const T*)this); }
    /// get context singleton, with default settings
    template<typename... Args>
    static T& get(Args&&... a) { return ContextMap::getContext().getDefault<T,T,Args...>(std::forward<Args>(a)...); }
    /// update from context singleton
    void lookup() { return ContextMap::getContext().lookup<T,T>(*(const T*)this); }
};

template<typename T>
struct s_context_singleton_ptr {
    /// Get current singleton instance
    static T* instance() { return ContextMap::getContext().get<s_context_singleton_ptr,T>(); }
protected:
    /// Register on construction
    s_context_singleton_ptr() {
        if(ContextMap::getContext().get<s_context_singleton_ptr,T>() != nullptr) throw std::runtime_error("Duplicate singleton instantiation");
        ContextMap::getContext().setPtr<s_context_singleton_ptr,T>(static_cast<T*>(this));
    }
    /// Deregister on destruction
    virtual ~s_context_singleton_ptr() { ContextMap::getContext().unset<s_context_singleton_ptr,T>(); }
};

/// "void" global object store helper
template<typename T>
T& GlobalContext() { return ContextMap::getContext().getDefault<void,T>(); }

/// string-tagged arguments
inline std::map<std::string, std::vector<std::string>>& GlobalArgs() { return GlobalContext<std::map<std::string, std::vector<std::string>>>(); }

/// load command-line arguments
void loadGlobalArgs(int argc, char** argv);
/// get number of times argument was specified
size_t numGlobalArg(const std::string& argname);
/// get required single-valued command line argument or throw error
const std::string& requiredGlobalArg(const std::string& argname);
/// pop one of multi-valued global arg (throw if none)
std::string popGlobalArg(const std::string& argname);
/// get optional argument with default
const std::string& optionalGlobalArg(const std::string& argname, const std::string& dflt = "");
/// update value with optional global floating-point argument
bool optionalGlobalArg(const std::string& argname, double& v);
/// update value with optional global floating-point argument
bool optionalGlobalArg(const std::string& argname, int& v);
/// debugging printout of global args
void displayGlobalArgs();

#endif
