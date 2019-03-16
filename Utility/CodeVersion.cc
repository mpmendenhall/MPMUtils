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

    const string compile_time = __DATE__ " " __TIME__;

#ifdef REPO_NAME
    const string repo_name = STRINGIFY(REPO_NAME);
#else
    const string repo_name = "Repo";
#endif

#ifdef REPO_VERSION
    const string repo_version = STRINGIFY(REPO_VERSION);
#else
    const string repo_version = "unknown";
#endif

#ifdef REPO_TAGNAME
    const string repo_tagname = STRINGIFY(REPO_TAGNAME);
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

    void display_code_version() {
        printf("%s '%s' (%s),\n compiled %s with %s (c++ %s) by %s@%s\n",
               repo_name.c_str(), repo_tagname.c_str(), repo_version.c_str(), compile_time.c_str(),
               compiler.c_str(), cpp_version.c_str(), user.c_str(), host.c_str());
    }
}
