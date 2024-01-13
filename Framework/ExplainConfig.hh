/// \file ExplainConfig.hh Verbose display of configuration file loading
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

/// Verbose query operations wrapper on settings group
class SettingsQuery: private NoCopy {
public:
    /// Constructor
    explicit SettingsQuery(const Setting& _S);
    /// Destructor
    ~SettingsQuery();

    /// Access associated setting
    operator const Setting& () const { return S; }
    /// Check if requested setting existed
    operator bool() const { return &S != &NullSetting; }

    /// Query setting existence; throw error if missing mandatory, else return silently
    bool exists(const string& name, const string& descrip = "", bool mandatory = false);

    /// Lookup setting, returning NullSetting if not present (or throwing error if mandatory)
    const Setting& lookup(const string& name, const string& descrip = "", bool mandatory = false) { return exists(name, descrip, mandatory)? S.lookup(name) : NullSetting; }

    /// Look up setting contents without printing description; return whether found
    template<class C>
    bool lookupValue(const string& name, C& val) { queried.insert(name); return S.lookupValue(name, val); }

    /// Describe optional-value lookup with default
    template<class C>
    bool lookupValue(const string& name, C& val, const string& descrip) {
        bool ex = show_exists(name, descrip);
        if(ex) {
            printf(TERMFG_BLUE "(default '%s')" TERMFG_GREEN " -> " TERMSGR_RESET "'" TERMSGR_BOLD TERMFG_MAGENTA, to_str(val).c_str());
            lookupValue(name, val);
        } else printf(TERMFG_GREEN "defaulted to " TERMSGR_RESET "'");
        printf("%s" TERMSGR_RESET "'\n", to_str(val).c_str());
        return ex;
    }

    /// Look up vector or single-value fill into size-n vector
    template<class C>
    bool lookupVector(const string& name, const string& descrip, vector<C>& v, size_t n, bool mandatory = false) {
        auto ex = show_exists(name, descrip, mandatory);
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

    /// Lookup one of multiple string choices
    bool lookupChoice(const string& name, string& val, const string& descrip, const set<string>& choices);
    /// Lookup and describe string-valued multiple choice option
    bool lookupChoice(const string& name, int& val, const string& descrip, const map<string, int>& choices);
    /// wrapper for enumeration-valued choices
    template<class C>
    bool lookupEnum(const string& name, C& val, const string& descrip, const map<string, C>& choices) {
        map<string, int> m;
        for(auto kv: choices) m.emplace(kv.first, kv.second);
        int i = val;
        auto b = lookupChoice(name, i, descrip, m);
        val = C(i);
        return b;
    }

    /// Get query-able subgroup or NullSetting;
    /// verbose terminal output if `descrip` supplied non-empty;
    /// error thrown if "mandatory" and not found
    SettingsQuery& get(const string& name, const string& descrip, bool mandatory = false);

    /// Level of panic in response to problematic issue
    enum Response_t {
        IGNORE, ///< silently ignore issue
        WARN,   ///< show warning about issue
        ERROR   ///< break with error on issue
    } require_queried; ///< whether to check/require all arguments are queried

    static Response_t default_require_queried;  ///< global default query requirement

    /// Helper to print location of setting in file
    static void printloc(const Setting& _S);

protected:
    /// return exists() + display setting description, "* Configuration ..." line or "*** Settings" group header
    bool show_exists(const string& name, const string& descrip, bool mandatory = false, bool header = false);

    /// Look up vector, or single value to fill into n-element vector. Returns empty vector if non-existent (or error if mandatory).
    vector<const Setting*> _lookupVector(const string& name, const string& descrip, size_t n, bool mandatory = false);

    const Setting& S;                   ///< settings group being queried
    set<string> queried;                ///< sub-settings (attempted) queried
    map<string, SettingsQuery> Ssub;    ///< sub-queries generated
};

#endif
