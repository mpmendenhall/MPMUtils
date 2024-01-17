/// @file

#include "ExplainConfig.hh"

using namespace ExplainConfig;

bool ExplainConfig::exists(const Setting& S, const string& name, const string& descrip, bool mandatory) {
     bool ex = S.exists(name);

    if(mandatory && !ex) {
        printf(TERMFG_RED "Required settings '%s' <%s> MISSING\nfrom ", name.c_str(), descrip.c_str());
        printloc(S);
        S.lookup(name); // throws missing setting exception
    }

    return ex;
}

/// print spaces to depth of setting
string pdepth(const Setting& _S, const string& s = "  ") {
    string d = "";
    const Setting* S = &_S;
    while(!S->isRoot()) {
        S = &S->getParent();
        d += s;
    }
    return d;
}

void ExplainConfig::printloc(const Setting& _S) {
    auto sf = _S.getSourceFile();
    auto p = _S.getPath();
    if(p.size()) printf("'%s' at ", p.c_str());
    printf("%s : line %i", sf? sf : "[in memory]", _S.getSourceLine());
}

/// return exists() + display setting description, "* Configuration ..." line or "*** Settings" group header
bool ExplainConfig::show_exists(const Setting& S, const string& name, const string& descrip, bool mandatory, bool header) {
    auto ex = exists(S, name, descrip, mandatory);

    if(mandatory) printf(TERMFG_MAGENTA);
    else printf(TERMFG_BLUE);
    auto pds = pdepth(S);
    auto pd = pds.c_str();

    if(header) {
        printf("\n%s**********************************************************\n%s**** " TERMSGR_RESET, pd, pd);
        if(mandatory) printf("Required ");
        printf("Settings '" TERMFG_GREEN "%s" TERMSGR_RESET "': %s ", name.c_str(), descrip.c_str());
        if(!ex) {
            printf(TERMFG_YELLOW "not provided\n" TERMFG_BLUE "%s**** within ", pd);
            printloc(S);
            printf(TERMSGR_RESET "\n");
        } else {
            printf(TERMFG_GREEN "provided" TERMSGR_RESET "\n");
            printf(TERMFG_BLUE "%s**** ", pd);
            printloc(S.lookup(name));
            printf(TERMSGR_RESET "\n");
        }
    } else {
        printf("%s*" TERMSGR_RESET " Configuration '" TERMFG_GREEN "%s" TERMSGR_RESET " <%s>' ",
               pd, name.c_str(), descrip.c_str());
    }

    return ex;
}

//---------------------------------------------------------------

SettingsQuery::Response_t SettingsQuery::default_require_queried = SettingsQuery::WARN;

SettingsQuery::SettingsQuery(const Setting& _S):
require_queried(default_require_queried), S(_S) { }

SettingsQuery::~SettingsQuery() {
    for(auto kv: Ssub) delete kv.second;

    // ignored, or nothing to be unused:
    if(!*this || require_queried == IGNORE) return;

    // skip checking unused if destructing during exception:
#if __cplusplus < 201703L
    if(std::uncaught_exception()) return;
#else
    if(std::uncaught_exceptions()) return;
#endif

    // check for unused items
    bool has_unused = false;
    for(auto& SS: S) {
        auto n = SS.getName();
        if(queried.count(n)) continue;

        if(require_queried == ERROR) printf(TERMFG_RED);
        else printf(TERMFG_YELLOW);
        printf("\n** Encountered unused configuration setting ");
        printloc(SS);
        printf("\n" TERMSGR_RESET);
        has_unused = true;
    }
    if(has_unused && require_queried == ERROR) std::terminate();
}

bool SettingsQuery::lookupChoice(const string& name, string& val, const string& descrip, const set<string>& choices, bool mandatory) {
    queried.insert(name);

    // update default if setting present
    bool ex = show_exists(name, descrip, mandatory, false);
    if(ex) {
        printf(TERMFG_BLUE "(default '%s') " TERMFG_GREEN "-> " TERMSGR_RESET, val.c_str());
        S.lookupValue(name, val);
        if(!choices.count(val)) {
            printf(TERMFG_RED "INVALID CHOICE '%s'\nfor ", val.c_str());
            printloc(S.lookup(name));
            printf("\n**** Allowed options:");
            for(const auto& c: choices) printf("\n  * '%s'", c.c_str());
            printf(TERMSGR_RESET "\n");
            throw std::runtime_error("invalid configuration selection");
        }
    } else printf(TERMFG_GREEN "defaulted to");

    // show selection in list of options
    for(const auto& c: choices) {
        if(c == val) {
            printf(TERMFG_YELLOW " *");
            if(ex) printf(TERMFG_MAGENTA TERMSGR_BOLD);
            else printf(TERMFG_GREEN);
        } else printf(" " TERMFG_BLUE);
        printf("'%s'" TERMSGR_RESET, c.c_str());
    }
    printf("\n");

    return ex;
}

vector<const Setting*> SettingsQuery::_lookupVector(const string& name, const string& descrip, size_t n, bool mandatory) {
    if(!exists(name, descrip, mandatory)) return {};

    vector<const Setting*> v;
    auto& SS = S.lookup(name);
    if(SS.isArray()) for(auto& i: SS) v.push_back(&i);
    else for(size_t i=0; i<n; ++i) v.push_back(&SS);
    return v;
}

bool SettingsQuery::lookupChoice(const string& name, int& val, const string& descrip, const map<string, int>& choices) {
    // identify default value
    auto it = choices.begin();
    while(it != choices.end()) {
        if(it->second == val) break;
        ++it;
    }
    if(it == choices.end()) throw std::logic_error("Invalid default value for choices");

    set<string> opts;
    for(auto& kv: choices) opts.insert(kv.first);
    auto s = it->first;
    bool ex = lookupChoice(name, s, descrip, opts);
    val = choices.at(s);
    return ex;
}

SettingsQuery& SettingsQuery::sub(const string& name) {
    exists(name, "", true);
    if(!Ssub.count(name)) Ssub.emplace(name, new SettingsQuery(S.lookup(name)));
    return *Ssub.at(name);
}

SettingsQuery& SettingsQuery::get(const string& name, const string& descrip, bool mandatory) {
    show_exists(name, descrip, mandatory, true);
    if(!Ssub.count(name)) Ssub.emplace(name, new SettingsQuery(lookup(name)));
    return *Ssub.at(name);
}
