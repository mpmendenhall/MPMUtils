/// \file libconfig_readerr.cc

#include "libconfig_readerr.hh"
#include <stdio.h>
#include <map>

std::map<const Setting*, const Config*> rConfigs;

const Setting& registerConfig(const Config& cfg) {
    auto& S = cfg.getRoot();
    rConfigs[&S] = &cfg;
    return S;
}

const Config* lookupConfig(const Setting* S) {
    if(!S) return nullptr;
    auto SS = const_cast<Setting*>(S);
    while(!SS->isRoot()) SS = &SS->getParent();
    auto it = rConfigs.find(SS);
    return it == rConfigs.end()? nullptr : it->second;
}

const Config& lookupConfig(const Setting& S) {
    auto c = lookupConfig(&S);
    if(!c) throw std::runtime_error("Request for unregistered Config");
    return *c;
}

const Config NullConfig;
const Setting& NullSetting = registerConfig(NullConfig);

void readConfigFile(Config& cfg, const string& cfgfile, bool autoconvert) {
    cfg.setAutoConvert(autoconvert);

    // set @include paths relative to config file instead of exec working dir
    auto i = cfgfile.find_last_of('/');
    if(i != string::npos) {
        auto cfgdir = cfgfile.substr(0,i+1);
        printf("Config base '%s'\n", cfgdir.c_str());
        cfg.setIncludeDir(cfgdir.c_str());
    }

    try { cfg.readFile(cfgfile.c_str()); }
    catch(ParseException& e) {
        printf("\n\nConfiguration file syntax error!\n");
        //auto ename = e.getError();
        //if(ename) printf("\t%s\n", ename);
        auto efile = e.getFile();
        if(efile) printf("In file: '%s' ", efile);
        printf("Line %i\n\n", e.getLine());
        fflush(stdout);
        throw;
    } catch(FileIOException& e) {
        printf("File I/O error loading config '%s'; check file exists and is readable!\n", cfgfile.c_str());
        fflush(stdout);
        throw;
    }
}

string cfgString(const Config& cfg) {
    char* buf = nullptr;
    size_t bsize = 1 << 15;
    FILE* f = open_memstream(&buf, &bsize);
    if(!f) throw std::runtime_error("open_memstream FAIL");
    cfg.write(f);
    auto n = ftell(f);
    rewind(f);
    string s(n, ' ');
    if(fread((void*)s.data(), 1, n, f) != size_t(n)) throw std::runtime_error("fread FAIL");
    fclose(f);
    free(buf);
    return s;
}
