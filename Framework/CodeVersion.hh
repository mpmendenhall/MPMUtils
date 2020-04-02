/// \file CodeVersion.hh Repository and compiler version info
// -- Michael P. Mendenhall, 2018

#ifndef CODEVERSION_HH
#define CODEVERSION_HH

#include <string>

/// Repository/compiler information
namespace CodeVersion {
    using std::string;

    extern const string repo_name;      ///< name of repository
    extern const string repo_version;   ///< repository version unique identifier
    extern const string repo_tagname;   ///< repository symbolic tag name
    extern const string compile_time;   ///< time of compile
    extern const string compiler;       ///< compiler name
    extern const string cpp_version;    ///< c++ standard version
    extern const string user;           ///< username of user running program
    extern const string host;           ///< hostname of computer running program

    /// code version descriptive string
    string description();
    /// print compile info to stdout
    void display_code_version();
}

#endif
