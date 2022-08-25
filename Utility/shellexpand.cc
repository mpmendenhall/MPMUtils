/// \file shellexpand.cc

#include "shellexpand.hh"
#include "to_str.hh"
#include <stdexcept>

vector<string> to_vs(wordexp_t w) {
    vector<string> v(w.we_wordc);
    for(size_t i = 0; i < w.we_wordc; ++i) v[i] = w.we_wordv[i + w.we_offs];
    wordfree(&w);
    return v;
}

vector<string> shellexpand(const string& s) {
    wordexp_t w = {0,nullptr,0};
    auto ret = wordexp(s.data(), &w, WRDE_SHOWERR | WRDE_UNDEF);
    if(ret) {
        wordfree(&w);
        if(ret == WRDE_BADCHAR) throw std::runtime_error("Disallowed character in expansion of '" + s + "'");
        if(ret == WRDE_BADVAL)  throw std::runtime_error("Undefined shell variable in expansion of '" + s + "'");
        if(ret == WRDE_SYNTAX)  throw std::runtime_error("Syntax error in expansion of '" + s + "'");
        throw std::runtime_error("wordexp(" + s + ") returned " + to_str(ret));
    }
    return to_vs(w);
}

string shellexpand_one(const string& s) {
    auto v = shellexpand(s);
    if(v.size() != 1) throw std::runtime_error("Shell expansion of '" + s + "' resulted in " + to_str(v.size()) + " strings; expected one");
    return v[0];
}
