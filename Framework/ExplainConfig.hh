/// @file ExplainConfig.hh Verbose display of configuration file loading
// -- Michael P. Mendenhall (LLNL) 2024

#ifndef EXPLAINCONFIG_HH
#define EXPLAINCONFIG_HH

#include "libconfig_readerr.hh"
#include "to_str.hh"
#include "TermColor.hh"
#include "NoCopy.hh"
#include <set>
using std::set;
#include <map>
using std::map;
using std::vector;

/// Standalone verbose setting query functions
namespace ExplainConfig {
    /// Query setting existence; throw error if missing mandatory, else return silently
    bool exists(const Setting& S, const string& name, const string& descrip = "", bool mandatory = false);

    /// Helper to print location of setting in file
    void printloc(const Setting& S);

    /// return exists() + display setting description, "* Configuration ..." line or "*** Settings" group header
    bool show_exists(const Setting& S, const string& name, const string& descrip, bool mandatory = false, bool header = true);

    /// Describe (optional)-value lookup with default
    template<class C>
    bool lookupValue(const Setting& S, const string& name, C& val, const string& descrip, bool mandatory = false) {
        bool ex = show_exists(S, name, descrip, mandatory, false);
        if(ex) {
            printf(TERMFG_BLUE "(default '%s')" TERMFG_GREEN " -> " TERMSGR_RESET "'" TERMSGR_BOLD TERMFG_MAGENTA, to_str(val).c_str());
            S.lookupValue(name, val);
        } else printf(TERMFG_GREEN "defaulted to " TERMSGR_RESET "'");
        printf("%s" TERMSGR_RESET "'\n", to_str(val).c_str());
        return ex;
    }
}

/// Verbose query operations wrapper on settings group, tracking used/unused settings
class SettingsQuery: private NoCopy {
public:
    /// Constructor, wrapping a `group` type setting (else error)
    SettingsQuery(const Setting& _S);
    /// Destructor; checks for un-queried settings in group
    ~SettingsQuery();


    //------------------------------------------------------
    // pass-through commands to underlying const Settings& S

    /// Query setting existence; throw error if missing mandatory, else return silently
    bool exists(const string& name, const string& descrip = "", bool mandatory = false) {
        queried.insert(name); return ExplainConfig::exists(S, name, descrip, mandatory);
    }
    /// Pass-through for getName()
    const char* getName() const { return S.getName(); }
    /// quiet mandatory setting lookup
    const Setting& operator[](const char* name) { queried.insert(name); return S[name]; }
    /// "dereference" to get underlying Setting
    const Setting& operator*() const { return S; }
    /// "dereference" to get underlying Setting functions
    const Setting* operator->() const { return &S; }

    //------------------------------------------------------------------------
    // "silent" operations without terminal output (aside from error messages)

    /// Mark as queried with no other actions
    void markused(const string& s) { queried.insert(s); }
    /// Check if requested setting existed
    operator bool() const { return &S != &NullSetting(); }

    /// Lookup setting, returning NullSetting if not present (or throwing error if mandatory)
    const Setting& lookup(const string& name, const string& descrip = "", bool mandatory = false) {
        return exists(name, descrip, mandatory)? S.lookup(name) : NullSetting();
    }
    /// Quiet loading of required subgroup
    SettingsQuery& sub(const string& name);

    //------------------------------------------------------------------------
    // "verbose" operations displaying status of found/default values

    /// return exists() + display setting description, "* Configuration ..." line or "*** Settings" group header
    bool show_exists(const string& name, const string& descrip, bool mandatory =  false, bool header = true) {
        queried.insert(name);
        return ExplainConfig::show_exists(S, name, descrip, mandatory, header);
    }

    /// Describe (optional)-value lookup with default
    template<class C>
    bool lookupValue(const string& name, C& val, const string& descrip = "", bool mandatory = false) {
        queried.insert(name);
        return ExplainConfig::lookupValue(S, name, val, descrip, mandatory);
    }

    /// Look up vector or single-value fill into size-n vector
    template<class C>
    bool lookupVector(const string& name, const string& descrip, vector<C>& v, size_t n, bool mandatory = false) {
        auto ex = show_exists(name, descrip, mandatory, false);
        if(ex) {
            printf(TERMFG_BLUE "(default '%s')" TERMFG_GREEN " -> " TERMSGR_RESET "'" TERMSGR_BOLD TERMFG_MAGENTA, to_str(v).c_str());

            auto vv = _lookupVector(name, descrip, n);
            v.resize(vv.size());
            auto it = v.begin();
            for(auto s: vv) *(it++) = C(*s);
        } else printf(TERMFG_GREEN "defaulted to " TERMSGR_RESET "'");

        printf("%s" TERMSGR_RESET "'\n", to_str(v).c_str());
        return ex;
    }

    /// Lookup one of multiple string choices, with `val` as default
    bool lookupChoice(const string& name, string& val, const string& descrip, const set<string>& choices, bool mandatory = false);
    /// Lookup and describe string-valued multiple choice option, mapped to integer for enum lookup
    bool lookupChoice(const string& name, int& val, const string& descrip, const map<string, int>& choices);
    /// Lookup and describe enumeration-valued string-named choices
    template<class C>
    bool lookupEnum(const string& name, C& val, const string& descrip, const map<string, C>& choices) {
        map<string, int> m;
        for(auto kv: choices) m.emplace(kv.first, kv.second);
        int i = val;
        auto b = lookupChoice(name, i, descrip, m);
        val = C(i);
        return b;
    }

    /// Get query-able subgroup or `NullSetting`.
    /** Verbose terminal output
     * error thrown if "mandatory" and not found.
     * Result is "owned" by this object, and will be destructed with unused query checking along with this.
     */
    SettingsQuery& get(const string& name, const string& descrip = "", bool mandatory = false);


    /// iterator through subnodes
    class iterator {
    public:
        /// constructor, for begin or end iterator
        explicit iterator(SettingsQuery& _SQ, bool start): SQ(_SQ), _it(start? SQ.S.begin() : SQ.S.end()) { }
        /// dereference to get contents
        SettingsQuery& operator*() { return SQ.sub(_it->getName()); }
        /// move to next
        iterator& operator++() { ++_it; return *this; }
        /// check if iterators unequal
        bool operator!=(const iterator& rhs) const { return _it != rhs._it; }

    protected:
        SettingsQuery& SQ;
        decltype(NullSetting().begin()) _it;    ///< underlying setting iterator
    };

    /// starting iterator
    iterator begin() { return iterator(*this, true); }
    /// ending iterator
    iterator end()   { return iterator(*this, false); }

    //------------------------------------------------
    // reaction to un-queried variables on destruction

    /// Level of panic in response to problematic issue
    enum Response_t {
        IGNORE, ///< silently ignore issue
        WARN,   ///< show warning about issue
        ERROR   ///< break with error on issue
    } require_queried; ///< whether to check/require all arguments are queried

    static Response_t default_require_queried;  ///< global default query requirement


protected:
    /// Look up vector, or single value to fill into n-element vector. Returns empty vector if non-existent (or error if mandatory).
    vector<const Setting*> _lookupVector(const string& name, const string& descrip, size_t n, bool mandatory = false);

    const Setting& S;                   ///< settings group being queried
    set<string> queried;                ///< sub-settings (attempted) queried
    map<string, SettingsQuery*> Ssub;   ///< sub-queries generated
};

#endif
