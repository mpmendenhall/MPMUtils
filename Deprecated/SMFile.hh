/// \file SMFile.hh simple text data format

#ifndef SMFile_HH
#define SMFile_HH

#include "Stringmap.hh"

/// wrapper for multimap<string,Stringmap> with useful functions
class SMFile: public multimap<string,Stringmap> {
public:
    /// Constructor with input filename
    explicit SMFile(const string& s = "", bool readit = true);

    /// retrieve values for key
    vector<Stringmap> retrieve(const string& s) const;
    /// retrieve first value for key
    Stringmap getFirst(const string& str, const Stringmap& dflt = Stringmap()) const;
    /// retrieve all sub-key values
    vector<string> retrieve(const string& k1, const string& k2) const;
    /// retreive sub-key with default
    string getDefault(const string& k1, const string& k2, const string& d) const;
    /// retrieve sub-key as double with default
    double getDefault(const string& k1, const string& k2, double d) const;
    /// retrieve all sub-key values as doubles
    vector<double> retrieveDouble(const string& k1, const string& k2) const;
    /// display to stdout
    void display() const;
};

#endif
