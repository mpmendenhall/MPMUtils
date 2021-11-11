/// \file MCTAL_Includes.hh Base includes for MCTAL file parser
// Michael P. Mendenhall, LLNL 2021

#ifndef MCTAL_INCLUDES_HH
#define MCTAL_INCLUDES_HH

#include <iostream>
using std::istream;
#include <string>
using std::string;
#include <vector>
using std::vector;
#include <set>
using std::set;
#include <stdexcept>
#include <algorithm>

/// particle type identifier
enum ptype_t {
    PTYPE_N   = 1,      ///< neutron
    PTYPE_G   = 2,      ///< gamma
    PTYPE_E   = 3,      ///< electron
    PTYPE_MUm = 4,      ///< mu-
    PTYPE_Nbar= 5,      ///< anti-neutron
    PTYPE_NUe = 6,      ///< electron neutrino
    PTYPE_NUm = 7,      ///< muon neutrino
    PTYPE_POSITRON = 8, ///< positron
    PTYPE_P   = 9,      ///< proton
    PTYPE_MUp = 16,     ///< mu+
    PTYPE_NUebar = 17,  ///< electron antineutrino
    PTYPE_2H  = 31,     ///< deuteron
    PTYPE_3H  = 32,     ///< triton
    PTYPE_3HE = 33,     ///< helion
    PTYPE_4HE = 34,     ///< alpha
    PTYPE_ION = 37,     ///< heavy ion
};
/// particles iteration helper
inline ptype_t& operator++(ptype_t& p) { return (p = ptype_t(p+1)); }

/// Check that string matches expectation
inline void check_expected(const string& sgot, const string& sexp) {
    if(sgot != sexp)
        throw std::runtime_error("Expected '"+ sexp +"', but got '"+ sgot +"'");
}

/// Check that character is in list
inline void check_expected(char c, const string& sexp) {
    for(auto cc: sexp) if(cc == c) return;
    char cc[2] = {c, 0};
    throw std::runtime_error("Expected char in ["+ sexp +"]; got '"+ cc +"'");
}

/// upperase string
inline string upper(string s) {
    std::transform(s.begin(), s.end(), s.begin(), (int(*)(int))toupper);
    return s;
}

#endif
