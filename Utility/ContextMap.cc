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
