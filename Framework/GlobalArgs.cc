/// \file GlobalArgs.cc

#include "GlobalArgs.hh"
#include "ContextMap.hh"
#include "TermColor.hh"
#include "to_str.hh"

struct gargmap: public std::map<string, std::vector<string>> { };
std::map<string, std::vector<string>>& GlobalArgs() {
    return ContextMap::getDefault<gargmap>();
}

struct qset: public std::set<string> { };
std::set<string>& QueriedArgs() {
    return ContextMap::getDefault<qset>();
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
    QueriedArgs().insert(argname);

    printf("* Argument '" TERMFG_GREEN "+%s" TERMSGR_RESET "' (%s) ", argname.c_str(), help.c_str());
    if(numGlobalArg(argname)) {
        printf(TERMFG_GREEN "enabled" TERMSGR_RESET "\n");
        return true;
    }
    printf(TERMFG_YELLOW "disabled" TERMSGR_RESET "\n");
    return false;
}

string requiredGlobalArg(const string& argname, const string& help) {
    QueriedArgs().insert(argname);

    printf(TERMFG_YELLOW "*" TERMSGR_RESET " Required argument '" TERMFG_GREEN "-%s" TERMSGR_RESET " <%s>' ", argname.c_str(), help.c_str());
    auto& v = GlobalArgs()[argname];
    if(v.size() != 1) {
        printf(TERMFG_RED "MISSING!" TERMSGR_RESET "\n");
        throw std::runtime_error("Expected one '-"+argname+"' argument");
    }
    printf(TERMFG_GREEN "-> '%s'" TERMSGR_RESET "\n", v[0].c_str());
    return v[0];
}

const vector<string>& requiredGlobalMulti(const string& argname, const string& help, size_t nmin) {
    QueriedArgs().insert(argname);

    auto& v = GlobalArgs()[argname];
    printf(TERMFG_YELLOW "*" TERMSGR_RESET " Required (at least %zu) argument '" TERMFG_GREEN "-%s"
    TERMSGR_RESET " <%s>' ", nmin, argname.c_str(), help.c_str());
    if(v.size() < nmin) {
        printf(TERMFG_RED "MISSING!" TERMSGR_RESET "\n");
        throw std::runtime_error("Expected at least " + to_str(nmin) + " '-"+argname+"' arguments, got " + to_str(v.size()));
    }
    printf(TERMFG_GREEN "->");
    for(auto& s: v) printf(" '%s'", s.c_str());
    printf(TERMSGR_RESET "\n");
    return v;
}

string popGlobalArg(const string& argname) {
    QueriedArgs().insert(argname);

    auto& v = GlobalArgs()[argname];
    if(!v.size()) throw std::runtime_error("Missing expected '-"+argname+"' argument");
    auto s = v.back();
    v.pop_back();
    return s;
}

string optionalGlobalDefault(const string& argname, const string& dflt, const string& help) {
    string s = dflt;
    optionalGlobalArg(argname, s, help);
    return s;
}

bool optionalGlobalArg(const string& argname, string& v, const string& help) {
    QueriedArgs().insert(argname);

    printf(TERMFG_BLUE "*" TERMSGR_RESET " Optional argument '" TERMFG_GREEN "-%s" TERMSGR_RESET " <%s>' ", argname.c_str(), help.c_str());
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
    QueriedArgs().insert(argname);

    auto& GA = GlobalArgs();
    auto it = GA.find(argname);
    bool noarg = it == GA.end() || !it->second.size();
    if(!noarg && it->second.size() > 1) throw std::runtime_error("Unexpected multiple '-"+argname+"' arguments");

    printf(TERMFG_BLUE "*" TERMSGR_RESET " Optional argument '" TERMFG_GREEN "+%s" TERMSGR_RESET "' (%s) ", argname.c_str(), help.c_str());
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

int checkUnusedArgs() {
    int unused = 0;
    auto& QA = QueriedArgs();
    for(auto& kv: GlobalArgs()) {
        if(QA.count(kv.first)) continue;
        printf(TERMFG_RED "* Unused command-line argument: " TERMFG_YELLOW "'%s'\n" TERMSGR_RESET, kv.first.c_str());
        ++unused;
    }
    return unused;
}
