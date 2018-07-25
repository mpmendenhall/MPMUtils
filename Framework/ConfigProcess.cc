/// \file ConfigProcess.cc

#include "ConfigProcess.hh"
#include <cassert>
#include <chrono>
using std::chrono::steady_clock;

REGISTER_MODULE(ConfigProcess,ConfigProcess)

ConfigProcess* ConfigProcess::construct(const Setting& S) {
    string n = "ConfigProcess";
    S.lookupValue("class", n);
    auto m = ModuleRegistrar<ConfigProcess>::construct(n);
    if(m) m->configure(S);
    else printf("Unknown module class '%s'!\n", n.c_str());
    return m;
}

void ConfigProcess::configure(const Setting& S) {
    S.lookupValue("verbose", verbose);
    S.lookupValue("class", name);
    _configure(S);
    if(S.exists("modules")) {
        for(auto& cfg: S["modules"]) {
            if(cfg.isList()) for(auto& c: cfg) add_module(c);
            else add_module(cfg);
        }
    }
    postconfig(S);
}

void ConfigProcess::finished(DataFrame& F, FrameSink& S) {
    auto it = stepnum.find(&F);
    if(it == stepnum.end()) {
        printf("ERROR '%s' got UNIDENTIFIED returned frame %p from '%s' with %i refs\n", name.c_str(), (void*)&F, S.name.c_str(), F.getRefs());
        throw;
    }
    if(verbose > 4) printf("'%s' got returned step-%zu frame %p from '%s' with %i refs\n", name.c_str(), it->second.first, (void*)&F, S.name.c_str(), F.getRefs());

    while(it->second.first < children.size() && !F.drop) {
        auto c = children[it->second.first++];
        if(verbose > 4) printf("%s sending frame %p to %s\n", name.c_str(), (void*)&F, c->name.c_str());
        auto t0 = steady_clock::now();
        c->receive(F,*this);
        c->timeUse.t_receive += std::chrono::duration<double>(steady_clock::now()-t0).count();
        if(c->keepsframe()) return;
    }

    assert(it->second.first == children.size() || F.drop);

    F.release();
    auto p = it->second.second;
    stepnum.erase(it);
    if(keepsframe() && (p != this)) p->finished(F,*this);
}

void ConfigProcess::receive(DataFrame& F, FrameSource& S) {
    if(verbose > 4) printf("%s received frame %p.\n", name.c_str(), (void*)&F);
    F.claim();
    stepnum.emplace(&F, std::make_pair(0,&S));
    finished(F,*this);
}

void ConfigProcess::add_module(const Setting& S) {
    auto m = construct(S);
    if(!m) {
        string n = "unknown";
        S.lookupValue("class", n);
        fprintf(stderr,"Unable to configure module class '%s'!\n", n.c_str());
        throw;
    }
    addChild(m);
    _keepsframe |= m->keepsframe();
}

void ConfigProcess::start_data(DataFrame& F) {
    F.claim();
    for(auto c: children) {
        auto t0 = steady_clock::now();
        c->start_data(F);
        c->timeUse.t_start += std::chrono::duration<double>(steady_clock::now()-t0).count();
    }
    F.release();
}
void ConfigProcess::end_data(DataFrame& F) {
    F.claim();
    for(auto c: children) {
        auto t0 = steady_clock::now();
        c->end_data(F);
        c->timeUse.t_start += std::chrono::duration<double>(steady_clock::now()-t0).count();
    }
    F.release();
    assert(!stepnum.size());
}

void ConfigProcess::borrow(void* o, DataFrame& F) {
    assert(!borrowed.count(o));
    borrowed[o] = &F;
    nborrowed[&F]++;
}

int ConfigProcess::release(void* o) {
    auto it = borrowed.find(o);
    assert(it != borrowed.end());
    auto F = it->second;
    borrowed.erase(it);

    auto it2 = nborrowed.find(F);
    assert(it2 != nborrowed.end());
    if(!(--(it2->second))) {
        nborrowed.erase(it2);
        doneBorrowing(*F);
        return 0;
    }
    return it2->second;
}

void ConfigProcess::doneBorrowing(DataFrame& F) {
    assert(!nborrowed.count(&F));
    assert(keepsframe());

    auto it = stepnum.find(&F);
    assert(it != stepnum.end());

    auto p = it->second.second;
    stepnum.erase(it);
    F.release();
    p->finished(F,*this);
}

void ConfigProcess::displayTimeSummary(int d) const {
    FrameSink::displayTimeSummary(d);
    profile_t ttl;
    for(auto c: children) {
        ttl += c->timeUse;
        c->displayTimeSummary(d+1);
    }
    if(children.size() > 1) {
        for(int i=0; i<=d; i++) printf("\t");
        printf("--------------------------------\n");
        for(int i=0; i<=d; i++) printf("\t");
        ttl.display();
        printf("\t: total\n");
    }
}
