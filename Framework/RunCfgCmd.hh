/// @file RunCfgCmd.hh Skeleton base executable for configuration-file-driven analysis
// Michael P. Mendenhall, LLNL 2021

#ifndef RUNCFGCMD_HH
#define RUNCFGCMD_HH

/// Skeleton base executable for configuration-file-driven analysis
class RunCfgCmd {
public:
    /// main() function for executable
    int main(int argc, char** argv, const char* execname);

    /// pre-run setup
    virtual void pre_run() { }
    /// post-run cleanup
    virtual void post_run() { }
};

#endif
