/// \file GlobalArgs.cc

#include "GlobalArgs.hh"
#include "ContextMap.hh"
#include "TermColor.hh"
#include "to_str.hh"

std::map<std::string, std::vector<std::string>>& GlobalArgs() {
    return ContextMap::getDefault<std::map<std::string, std::vector<std::string>>>();
}

void loadGlobalArgs(int argc, char** argv) {
    auto& GA = GlobalArgs();

    for(int c = 0; c < argc; ++c) {
        if(argv[c][0] == '+') {
            GA[argv[c] + 1].push_back("y");
            continue;
        }

        if(argv[c][0] != '-') throw std::runtime_error("Arguments syntax glitch at '" + std::string(argv[c]) +"'");
        if(c == argc-1) throw std::runtime_error("Missing value for final argument '" + std::string(argv[c]) +"'");

        auto& v = GA[argv[c] + 1];
        do { v.emplace_back(argv[++c]); }
        while(c+1 < argc && argv[c+1][0] != '-' && argv[c+1][0] != '+');
    }
}

bool string_to_bool(const string& s) {
    if(!s.size()) return false;
    if(atoi(s.c_str())) return true;
    auto c = s[0];
    return c=='Y' || c == 'y' || c == 'T' || c == 't';
}

size_t numGlobalArg(const string& argname) {
    auto it = GlobalArgs().find(argname);
    return it == GlobalArgs().end()? 0 : it->second.size();
}

bool wasArgGiven(const string& argname, const string& help) {
    printf("* Argument '" TERMFG_GREEN "+%s" TERMSGR_RESET "' [%s] ", argname.c_str(), help.c_str());
    if(numGlobalArg(argname)) {
        printf(TERMFG_GREEN "enabled" TERMSGR_RESET "\n");
        return true;
    }
    printf(TERMFG_YELLOW "disabled" TERMSGR_RESET "\n");
    return false;
}

const string& requiredGlobalArg(const string& argname, const string& help) {
    printf("* Required argument " TERMFG_GREEN "-%s" TERMSGR_RESET "' <%s>' ", argname.c_str(), help.c_str());
    auto& v = GlobalArgs()[argname];
    if(v.size() != 1) {
        printf(TERMFG_GREEN "MISSING!" TERMSGR_RESET "\n");
        throw std::runtime_error("Expected one '-"+argname+"' argument");
    }
    printf(TERMFG_GREEN "-> '%s'" TERMSGR_RESET "\n", v[0].c_str());
    return v[0];
}

string popGlobalArg(const string& argname) {
    auto& v = GlobalArgs()[argname];
    if(!v.size()) throw std::runtime_error("Missing expected '-"+argname+"' argument");
    auto s = v.back();
    v.pop_back();
    return s;
}

bool optionalGlobalArg(const string& argname, string& v, const string& help) {
    printf("* Optional argument '" TERMFG_GREEN "-%s" TERMSGR_RESET "' <%s> ", argname.c_str(), help.c_str());
    auto& GA = GlobalArgs();
    auto it = GA.find(argname);
    if(it == GA.end() || !it->second.size()) {
        printf(TERMFG_GREEN "defaulted to" TERMSGR_RESET " '%s'\n", v.c_str());
        return false;
    }
    if(it->second.size() > 1) throw std::runtime_error("Unexpected multiple '-"+argname+"' arguments");
    v = it->second[0];
    printf(TERMFG_GREEN "-> '%s'" TERMSGR_RESET "\n", v.c_str());
    return true;
}

bool optionalGlobalArg(const string& argname, double& v, const string& help) {
    string s = to_str(v);
    if(!optionalGlobalArg(argname, s, help)) return false;
    v = atof(s.c_str());
    return true;
}

bool optionalGlobalArg(const string& argname, int& v, const string& help) {
    string s = to_str(v);
    if(!optionalGlobalArg(argname, s, help)) return false;
    v = atoi(s.c_str());
    return true;
}

bool optionalGlobalArg(const string& argname, bool& v, const string& help) {
    auto& GA = GlobalArgs();
    auto it = GA.find(argname);
    bool noarg = it == GA.end() || !it->second.size();
    if(!noarg && it->second.size() > 1) throw std::runtime_error("Unexpected multiple '-"+argname+"' arguments");

    printf("* Optional argument '" TERMFG_GREEN "+%s" TERMSGR_RESET "' <%s> ", argname.c_str(), help.c_str());
    if(noarg) {
        printf(TERMFG_GREEN "defaulted to ");
    } else {
        v = string_to_bool(it->second[0]);
        printf(TERMFG_GREEN "-> ");
    }
    printf(TERMSGR_RESET "'");
    if(v) printf(TERMFG_GREEN "true");
    else printf(TERMFG_YELLOW "false");
    printf(TERMSGR_RESET "'\n");
    return !noarg;
}

void displayGlobalArgs() {
    printf("Global Arguments:\n");
    for(auto& kv: GlobalArgs()) {
        printf("'%s':\n", kv.first.c_str());
        for(auto& s: kv.second) printf("\t* '%s'\n", s.c_str());
    }
}
