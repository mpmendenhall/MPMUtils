/// \file CodeVersion.cc

#include "CodeVersion.hh"
#include <unistd.h> // for gethostname
#include <pwd.h>    // for user name
#include <stdio.h>  // for printf

/// convert literal text to const char* (no #define expansion)
#define STRINGIFY_VERBATIM(...) #__VA_ARGS__
/// convert text to const char* after #define expansion
#define STRINGIFY(X) STRINGIFY_VERBATIM(X)

namespace CodeVersion {

    string strip_quotes(const string& s) {
        return (s[0] != '"' || *s.rbegin() != '"')? s : s.substr(1, s.size()-2);
    }

    const string compile_time = __DATE__ " " __TIME__;

#ifdef REPO_NAME
    const string repo_name = strip_quotes(STRINGIFY(REPO_NAME));
#else
    const string repo_name = "Repo";
#endif

#ifdef REPO_VERSION
    const string repo_version = strip_quotes(STRINGIFY(REPO_VERSION));
#else
    const string repo_version = "unknown";
#endif

#ifdef REPO_TAGNAME
    const string repo_tagname = strip_quotes(STRINGIFY(REPO_TAGNAME));
#else
    const string repo_tagname = "unknown";
#endif

#ifdef __GNUC__
    const string compiler = STRINGIFY(gcc __GNUC__.__GNUC_MINOR__.__GNUC_PATCHLEVEL__);
#else
#ifdef __VERSION__
    const string compiler = STRINGIFY(__VERSION__);
#else
    const string compiler = "unknown";
#endif
#endif

#ifdef __cplusplus
    const string cpp_version = STRINGIFY(__cplusplus);
#else
    const string cpp_version = "unknown";
#endif

    const char* get_hostname() {
        auto c = new char[1025];
        c[1024] = 0; // assure array terminated if hostname truncated
        if(!gethostname(c,1024)) return c;
        return "";
    }
    const string host = get_hostname();

    const char* get_user() {
        auto uid = getuid();
        auto p = getpwuid(uid);
        if(p && p->pw_name) return p->pw_name;
        return "";
    }
    const string user = get_user();

    string description() {
        return (repo_name + " '" + repo_tagname + "' (" + repo_version + "),\n compiled "
                + compile_time + " with " + compiler + " (c++ " + cpp_version
                + ") by " + user + "@" + host);
    }

    void display_code_version() {
        printf("%s\n", description().c_str());
        fflush(stdout);
    }
}
