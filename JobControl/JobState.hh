/// @file JobState.hh Utility for storage and retrieval of hash-identified state job state information

#ifndef JOBSTATE_HH
#define JOBSTATE_HH

#include "KeyTable.hh"

/// Utility for storage and retrieval of hash-identified state job state information
class JobState {
public:
    /// polymorphic destructor
    virtual ~JobState() { }

    /// check if state data available (and make available if possible) for hash
    virtual bool checkState(const string& h);
    /// clear state data for hash
    virtual void clearState(const string& h);

    /// push state data for identifier hash
    template<class T>
    void pushState(const string& h, const T& d) {
        stateData.emplace(h,d);
        persistState(h);
    }

    /// load state data for identifier hash
    template<class T>
    void getState(const string& h, T& d) {
        if(!checkState(h)) throw std::range_error("State data unavailable");
        stateData.at(h).Get(d);
    }

    static string stateDir;         ///< non-empty to specify directory for state data storage

protected:
    /// name for state data file
    virtual string sdataFile(const string& h) const;
    /// persistently save state data for hash
    virtual void persistState(const string& h);

    map<string, KeyData> stateData; ///< memory-resident saved state information by hash
    map<string, size_t> lastReq;    ///< when each piece of stored data was last requested
    size_t nReq = 0;                ///< number of times stored data has been requested
};

#endif
