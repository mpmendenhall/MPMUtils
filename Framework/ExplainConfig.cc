/// \file ExplainConfig.cc

#include "ExplainConfig.hh"

bool SettingsQuery::default_require_queried = true;

SettingsQuery::SettingsQuery(const Setting& _S):
require_queried(default_require_queried), S(_S) {
    if(*this && !S.isGroup())
        throw std::runtime_error("Setting '" + S.getPath() + "' must be of { group } type");
}

SettingsQuery::~SettingsQuery() {
    if(!*this || !require_queried) return;
    for(auto& SS: S) {
        auto n = SS.getName();
        if(queried.count(n)) continue;

        auto sf = S.getSourceFile();
        printf(TERMFG_RED "\n** Encountered unused configuration setting '%s'\n** in %s : line %i\n" TERMSGR_RESET,
               SS.getPath().c_str(), sf? sf : "<no file>", SS.getSourceLine());
        std::terminate();
    }
}

void _printloc(const Setting& S) {
    auto sf = S.getSourceFile();
    auto p = S.getPath();
    if(p.size()) printf("'%s' at ", p.c_str());
    printf("%s : line %i", sf? sf : "<no file>", S.getSourceLine());
}

void SettingsQuery::printloc() const { _printloc(S); }

bool SettingsQuery::lookupChoice(const string& name, int& val, const string& descrip, const map<string, int>& choices) {

    queried.insert(name);

    // identify default value
    auto it = choices.begin();
    while(it != choices.end()) {
        if(it->second == val) break;
        ++it;
    }
    if(it == choices.end()) throw std::logic_error("Invalid default value for choices");

    // update default if setting present
    bool ex = _checkOpt(name, descrip);
    if(ex) {
        printf(TERMFG_BLUE "(default '%s') " TERMFG_GREEN "-> " TERMSGR_RESET, it->first.c_str());
        string s = S[name];
        it = choices.find(s);
        if(it == choices.end()) {
            printf(TERMFG_RED "INVALID CHOICE '%s'\nfor ", s.c_str());
            _printloc(S[name]);
            printf("\n**** Allowed options:");
            for(auto kv: choices) printf("\n  * '%s'", kv.first.c_str());
            printf(TERMSGR_RESET "\n");
            throw std::runtime_error("invalid configuration selection");
        }
        val = it->second;
    } else printf(TERMFG_GREEN "defaulted to");

    // show selection in list of options
    for(auto& kv: choices) {
        if(kv.second == val) {
            printf(TERMFG_YELLOW " *");
            if(ex) printf(TERMFG_MAGENTA TERMSGR_BOLD);
            else printf(TERMFG_GREEN);
        } else printf(" " TERMFG_BLUE);
        printf("'%s'" TERMSGR_RESET, kv.first.c_str());
    }
    printf("\n");

    return ex;
}

SettingsQuery& SettingsQuery::get(const string& name, const string& descrip, bool mandatory) {
    if(!Ssub.count(name)) Ssub.emplace(name, lookupOpt(name));

    auto& SQ = Ssub.at(name);
    if(!descrip.size()) {
        if(!SQ && mandatory) S[name]; // throws missing setting exception
        return SQ;
    }
    printf(TERMFG_BLUE "\n**********************************************************\n**** " TERMSGR_RESET);
    if(mandatory) printf("Required ");
    printf("Settings group '" TERMFG_GREEN "%s" TERMSGR_RESET "': %s ", name.c_str(), descrip.c_str());
    if(!SQ) {
        if(mandatory) {
            printf(TERMFG_RED "MISSING" TERMSGR_RESET "\n");
            S[name]; // throws missing setting exception
        }
        printf(TERMFG_YELLOW "not provided; using defaults\n" TERMFG_BLUE "**** within ");
        printloc();
        printf(TERMSGR_RESET "\n");
    } else {
        printf(TERMFG_GREEN "provided\n" TERMFG_BLUE "**** ");
        SQ.printloc();
        printf(TERMSGR_RESET "\n");
    }
    return SQ;
}

bool SettingsQuery::_checkOpt(const string& name, const string& descrip) const {
    printf(TERMFG_BLUE "*" TERMSGR_RESET " Configuration '");
    printf(TERMFG_GREEN "%s" TERMSGR_RESET " <%s>' ", name.c_str(), descrip.c_str());
    return S.exists(name);
}
