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
    wordexp_t w;
    w.we_wordc = w.we_offs = 0;
    auto ret = wordexp(s.data(), &w, WRDE_SHOWERR | WRDE_UNDEF);
    if(ret) {
        wordfree(&w);
        throw std::runtime_error("wordexp returned " + to_str(ret));
    }
    return to_vs(w);
}

string shellexpand_one(const string& s) {
    auto v = shellexpand(s);
    if(v.size() != 1) throw std::runtime_error("Shell expansion of '" + s + "' resulted in " + to_str(v.size()) + " strings; expected one");
    return v[0];
}
