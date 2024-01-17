/// @file

#include "libconfig_readerr.hh"
#include <stdio.h>
#include <map>
#include <stdexcept>

std::map<const Setting*, const Config*>& rConfigs() {
    static std::map<const Setting*, const Config*> m;
    return m;
}

const Setting& registerConfig(const Config& cfg) {
    auto& S = cfg.getRoot();
    rConfigs()[&S] = &cfg;
    return S;
}

const Config* lookupConfig(const Setting* S) {
    if(!S) return nullptr;
    auto SS = const_cast<Setting*>(S);
    while(!SS->isRoot()) SS = &SS->getParent();
    auto it = rConfigs().find(SS);
    return it == rConfigs().end()? nullptr : it->second;
}

const Config& lookupConfig(const Setting& S) {
    auto c = lookupConfig(&S);
    if(!c) throw std::runtime_error("Request for unregistered Config");
    return *c;
}

const Config& NullConfig() {
    static Config C;
    return C;
}

const Setting& NullSetting() {
    static auto& S = registerConfig(NullConfig());
    return S;
}

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
    const size_t bsize = 1 << 15;
    char buf[bsize];
    FILE* f = fmemopen(buf, bsize, "r+");
    if(!f) throw std::runtime_error("fmemopen FAIL");

    cfg.write(f);
    auto n = ftell(f);
    rewind(f);
    string s(n, ' ');
    if(fread(&s[0], 1, n, f) != size_t(n)) throw std::runtime_error("fread FAIL");
    fclose(f);

    return s;
}
