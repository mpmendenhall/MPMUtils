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

/// Verbose query operations wrapper on settings group
class SettingsQuery: private NoCopy {
public:
    /// Constructor
    SettingsQuery(const Setting& _S);
    /// Destructor
    ~SettingsQuery();

    /// Access associated setting
    operator const Setting& () const { return S; }
    /// Check if requested setting existed
    operator bool() const { return &S != &NullSetting; }

    /// Query setting existence; verbose if non-empty descrip provided
    bool exists(const string& name, const string& descrip = "", bool mandatory = false);
    /// Lookup optional, returning NullSetting if not present
    const Setting& lookupOpt(const string& name) { return exists(name)? S[name] : NullSetting; }
    /// Mandatory setting lookup
    Setting& lookupReq(const string& name) { queried.insert(name); return S[name]; }

    /// Look up setting contents without printing description; return whether found
    template<class C>
    bool lookupValue(const string& name, C& val) { queried.insert(name); return S.lookupValue(name, val); }
    /// Describe optional-value lookup with default
    template<class C>
    bool lookupValue(const string& name, C& val, const string& descrip) {
        bool ex = _checkOpt(name, descrip);
        if(ex) {
            printf(TERMFG_BLUE "(default '%s')" TERMFG_GREEN " -> " TERMSGR_RESET "'" TERMSGR_BOLD TERMFG_MAGENTA, to_str(val).c_str());
            lookupValue(name, val);
        } else printf(TERMFG_GREEN "defaulted to " TERMSGR_RESET "'");
        printf("%s" TERMSGR_RESET "'\n", to_str(val).c_str());
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

    static bool default_require_queried;    ///< global default query requirement
    bool require_queried;                   ///< whether to check/require all arguments are queried


protected:
    /// Optional-value lookup
    bool _checkOpt(const string& name, const string& descrip) const;
    /// Print path / file location
    void printloc() const;

    const Setting& S;                   ///< settings group being queried
    set<string> queried;                ///< sub-settings (attempted) queried
    map<string, SettingsQuery> Ssub;    ///< sub-queries generated
};

#endif
