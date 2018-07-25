/// \file CodeVersion.hh Repository and compiler version info
// -- Michael P. Mendenhall, 2018

#ifndef CODEVERSION_HH
#define CODEVERSION_HH

#include <string>

/// Repository/compiler information
namespace CodeVersion {
    using std::string;

    extern const string repo_version;   ///< repository version unique identifier
    extern const string repo_tagname;   ///< repository symbolic tag name
    extern const string compile_time;   ///< time of compile
    extern const string compiler;       ///< compiler name

    /// print compile info to stdout
    void display_code_version();
}

#endif
