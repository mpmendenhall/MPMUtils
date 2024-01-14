/// @file AnalysisStep.hh "Analysis step" XML output wrapper
// -- Michael P. Mendenhall, LLNL 2019

#ifndef ANALYSISSTEP_HH
#define ANALYSISSTEP_HH

#include "ContextMap.hh"
#include "XMLTag.hh"
#include "CodeVersion.hh"
#include <time.h>
#include <chrono>
using std::chrono::steady_clock;
typedef std::chrono::time_point<steady_clock> sclock_time;

/// get md5sum hash for file
string md5sum(const string& f);

/// generate input file XML entry, with md5 of the file (use on '.xml' for large files!)
XMLTag* infileEntry(const string& f);

/// Base class for setting up standard ``analysis step'' .xml metadata
class AnalysisStep: public XMLProvider, public s_context_singleton_ptr<AnalysisStep> {
public:
    /// Constructor
    explicit AnalysisStep(const string& cd);

    vector<string> infiles;     ///< list of input files
    string outfilename;         ///< output file name
    string codename;            ///< name of this analysis code
    time_t t0;                  ///< start of process (UNIX timestamp)
    sclock_time pt0;            ///< precision start time

    string anatag;              ///< outermost tag for analysis

    /// initialize output XML file by copy from first input file
    void make_xmlout();

protected:
    /// construct XML output
    void _makeXML(XMLTag& X) override;
};

#endif
