/// @file to_str.hh Utility template for anything-to-string

#ifndef TO_STR_HH
#define TO_STR_HH

#include <sstream>
#include <string>
using std::string;
#include <vector>
using std::vector;
#include <stdexcept>
#include <cstdio>   // for vsnprintf
#include <stdarg.h> // for variadic args passing

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
    string u = "[ ";
    for(const auto& x: v) u += to_str(x) + " ";
    return u + "]";
}

/// printf(...) functionality for string
inline string string_format(string fmt, ...) {
    va_list args, args2;
    va_start(args, fmt);
    va_copy(args2, args);
    auto n = vsnprintf(nullptr, 0, fmt.c_str(), args); // get formatted size
    va_end(args);
    if(n < 0) {
        va_end(args2);
        throw std::runtime_error("printf() format error");
    }

    vector<char> v(n+1); // one larger for \0 termination
    vsnprintf(v.data(), n+1, fmt.c_str(), args2);
    va_end(args2);
    return string(v.data(), v.data() + n);
}

#endif
