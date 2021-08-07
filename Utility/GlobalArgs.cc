/// \file GlobalArgs.cc

#include "GlobalArgs.hh"
#include "ContextMap.hh"
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

size_t numGlobalArg(const std::string& argname) {
    auto it = GlobalArgs().find(argname);
    return it == GlobalArgs().end()? 0 : it->second.size();
}

bool wasArgGiven(const std::string& argname, const std::string& help) {
    printf("* Argument '+%s' [%s] ", argname.c_str(), help.c_str());
    if(numGlobalArg(argname)) {
        printf("enabled\n");
        return true;
    }
    printf("disabled\n");
    return false;
}

const std::string& requiredGlobalArg(const std::string& argname, const std::string& help) {
    printf("* Required argument '-%s <%s>' ", argname.c_str(), help.c_str());
    auto& v = GlobalArgs()[argname];
    if(v.size() != 1) {
        printf("MISSING!\n");
        throw std::runtime_error("Expected one '-"+argname+"' argument");
    }
    printf("-> '%s'\n", v[0].c_str());
    return v[0];
}

std::string popGlobalArg(const std::string& argname) {
    auto& v = GlobalArgs()[argname];
    if(!v.size()) throw std::runtime_error("Missing expected '-"+argname+"' argument");
    auto s = v.back();
    v.pop_back();
    return s;
}

bool optionalGlobalArg(const std::string& argname, std::string& v, const std::string& help) {
    printf("* Optional argument '-%s <%s>' ", argname.c_str(), help.c_str());
    auto& GA = GlobalArgs();
    auto it = GA.find(argname);
    if(it == GA.end() || !it->second.size()) {
        printf("defaulted to '%s'\n", v.c_str());
        return false;
    }
    if(it->second.size() > 1) {
        printf(" specified too many times!\n");
        throw std::runtime_error("Unexpected multiple '-"+argname+"' arguments");
    }
    v = it->second[0];
    printf("-> '%s'\n", v.c_str());
    return true;
}

bool optionalGlobalArg(const std::string& argname, double& v, const string& help) {
    string s = to_str(v);
    if(!optionalGlobalArg(argname, s, help)) return false;
    v = atof(s.c_str());
    return true;
}

bool optionalGlobalArg(const std::string& argname, int& v, const string& help) {
    string s = to_str(v);
    if(!optionalGlobalArg(argname, s, help)) return false;
    v = atoi(s.c_str());
    return true;
}

void displayGlobalArgs() {
    printf("Global Arguments:\n");
    for(auto& kv: GlobalArgs()) {
        printf("'%s':\n", kv.first.c_str());
        for(auto& s: kv.second) printf("\t* '%s'\n", s.c_str());
    }
}
