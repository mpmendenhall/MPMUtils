/// \file ContextMap.cc

#include "ContextMap.hh"

std::vector<ContextMap*>& ContextMap::getContextStack() {
    static thread_local std::vector<ContextMap*> v;
    return v;
}

ContextMap& ContextMap::getContext() {
    auto& v = getContextStack();
    if(!v.size()) v.push_back(new ContextMap);
    return *v.back();
}

ContextMap& ContextMap::pushContext() {
    auto& v = getContextStack();
    v.push_back(new ContextMap(v.size()? v.back() : nullptr));
    return *v.back();
}

bool ContextMap::popContext() {
    auto& v = getContextStack();
    if(v.size()) {
        delete v.back();
        v.pop_back();
        return true;
    }
    return false;
}

ContextMap& ContextMap::operator=(const ContextMap& M) {
    if(&M == this) return *this;
    for(auto& kv: M.dat) {
        if(kv.second.second) dat[kv.first] = {kv.second.second->clone(kv.second.first), kv.second.second->clowner()};
        else dat.emplace(kv);
    }
    return *this;
}

void ContextMap::disown(tp_t x) {
    auto it = dat.find(x);
    if(it == dat.end()) return;
    if(it->second.second) it->second.second->deletep(it->second.first);
    dat.erase(it);
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

const std::string& requiredGlobalArg(const std::string& argname) {
    auto& v = GlobalArgs()[argname];
    if(v.size() != 1) throw std::runtime_error("Expected one '-"+argname+"' argument");
    return v[0];
}

const std::string& optionalGlobalArg(const std::string& argname, const std::string& dflt) {
    auto& GA = GlobalArgs();
    auto it = GA.find(argname);
    if(it == GA.end() || !it->second.size()) return dflt;
    if(it->second.size() > 1) throw std::runtime_error("Unexpected multiple '-"+argname+"' arguments");
    return it->second[0];
}

void displayGlobalArgs() {
    printf("Global Arguments:\n");
    for(auto& kv: GlobalArgs()) {
        printf("'%s':\n", kv.first.c_str());
        for(auto& s: kv.second) printf("\t* '%s'\n", s.c_str());
    }
}
