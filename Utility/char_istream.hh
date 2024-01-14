/// @file char_istream.hh istream using supplied const char buffer
// Michael P. Mendenhall, LLNL 2021

#ifndef CHAR_ISTREAM_HH
#define CHAR_ISTREAM_HH

#include <istream>
#include <string>
using std::string;

/// wrapper for char array to stream buffer
struct char_membuf: std::basic_streambuf<char> {
    /// default constructor from char* array
    explicit char_membuf(const char* s = nullptr, size_t n = 0) { set(s,n); }
    /// construct from string
    explicit char_membuf(const string& s) { set(s); }

    /// set buffer
    void set(const char* s, size_t n) {
        auto p = const_cast<char*>(s);
        setg(p, p, p+n);
    }
    /// set buffer from std::string
    void set(const string& s) { set(s.c_str(), s.size()); }
};

/// istream over char buffer
class char_istream: private char_membuf, public std::basic_istream<char> {
public:
    /// Constructor
    explicit char_istream(const char* s = nullptr, size_t n = 0):
    char_membuf(s,n), basic_istream(this) { }

    /// Set contents
    void set_str(const char* s, size_t n) { set(s,n); init(this); }
    /// Set contents
    void set_str(const string& s) { set_str(s.c_str(), s.size()); }
};

#endif
