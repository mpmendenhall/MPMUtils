/// \file GetEnv.cc
// -- Michael P. Mendenhall, LLNL 2019

#include "GetEnv.hh"
#include <unistd.h>
#include <stdexcept>

bool getEnv(const string& name, string& val, bool fail_if_missing) {
    const char* envv = getenv(name.c_str());
    if(envv) {
        val = envv;
        return true;
    } else if(fail_if_missing)
        throw std::runtime_error("Missing environment variable '"+name+"'");
    return false;
}

string getEnv(const string& name, bool fail_if_missing) {
    const char* envv = getenv(name.c_str());
    if(envv) return envv;
    if(fail_if_missing) throw std::runtime_error("Missing environment variable '"+name+"'");
    return "";
}


