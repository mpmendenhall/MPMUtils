/// \file to_str.hh Utility template for anything-to-string

#ifndef TO_STR_HH
#define TO_STR_HH

#include <sstream>
#include <string>
using std::string;
#include <vector>
using std::vector;

/// utility function for converting to string
template<typename T>
inline string to_str(const T& x) {
    std::stringstream ss;
    ss << x;
    return ss.str();
}

/// utility for displaying vector as string
template<typename T>
inline string to_str(const vector<T>& v) {
    string s = "[ ";
    for(auto& x: v) s += to_str(x) + " ";
    return s + "]";
}

#endif
