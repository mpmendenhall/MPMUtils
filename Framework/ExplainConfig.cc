/// @file

#include "ExplainConfig.hh"

SettingsQuery::Response_t SettingsQuery::default_require_queried = SettingsQuery::WARN;

SettingsQuery::SettingsQuery(const Setting& _S):
require_queried(default_require_queried), S(_S) {
    if(*this && !S.isGroup())
        throw std::runtime_error("Setting '" + S.getPath() + "' must be of { group } type");
}

void SettingsQuery::printloc(const Setting& _S) {
    auto sf = _S.getSourceFile();
    auto p = _S.getPath();
    if(p.size()) printf("'%s' at ", p.c_str());
    printf("%s : line %i", sf? sf : "<no file>", _S.getSourceLine());
}

SettingsQuery::~SettingsQuery() {
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

bool SettingsQuery::lookupChoice(const string& name, string& val, const string& descrip, const set<string>& choices) {
    // update default if setting present
    bool ex = show_exists(name, descrip);
    if(ex) {
        printf(TERMFG_BLUE "(default '%s') " TERMFG_GREEN "-> " TERMSGR_RESET, val.c_str());
        S.lookupValue(name, val);
        if(!choices.count(val)) {
            printf(TERMFG_RED "INVALID CHOICE '%s'\nfor ", val.c_str());
            printloc(S.lookup(name));
            printf("\n**** Allowed options:");
            for(auto& c: choices) printf("\n  * '%s'", c.c_str());
            printf(TERMSGR_RESET "\n");
            throw std::runtime_error("invalid configuration selection");
        }
    } else printf(TERMFG_GREEN "defaulted to");

    // show selection in list of options
    for(auto& c: choices) {
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

bool SettingsQuery::exists(const string& name, const string& descrip, bool mandatory) {
    queried.insert(name);
    bool ex = S.exists(name);

    if(mandatory && !ex) {
        printf(TERMFG_RED "Required settings '%s' <%s> MISSING\nfrom ", name.c_str(), descrip.c_str());
        printloc(S);
        S.lookup(name); // throws missing setting exception
    }

    return ex;
}

bool SettingsQuery::show_exists(const string& name, const string& descrip, bool mandatory, bool header) {
    auto ex = exists(name, descrip, mandatory);

    if(mandatory) printf(TERMFG_MAGENTA);
    else printf(TERMFG_BLUE);

    if(header) {
        printf("\n**********************************************************\n**** " TERMSGR_RESET);
        if(mandatory) printf("Required ");
        printf("Settings '" TERMFG_GREEN "%s" TERMSGR_RESET "': %s ", name.c_str(), descrip.c_str());
        if(!ex) {
            printf(TERMFG_YELLOW "not provided\n" TERMFG_BLUE "**** within ");
            printloc(S);
            printf(TERMSGR_RESET "\n");
        } else {
            printf(TERMFG_GREEN "provided" TERMSGR_RESET "\n" TERMFG_BLUE "**** ");
            printloc(S.lookup(name));
            printf(TERMSGR_RESET "\n");
        }
    } else {
        printf("*" TERMSGR_RESET " Configuration '" TERMFG_GREEN "%s" TERMSGR_RESET " <%s>' ",
               name.c_str(), descrip.c_str());
    }

    return ex;
}

SettingsQuery& SettingsQuery::get(const string& name, const string& descrip, bool mandatory) {
    show_exists(name, descrip, mandatory, true);
    if(!Ssub.count(name)) Ssub.emplace(name, lookup(name));
    return Ssub.at(name);
}
