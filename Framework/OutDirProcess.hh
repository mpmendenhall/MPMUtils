/// \file OutDirProcess.hh ConfigProcess associated with a TObject output directory
// -- Michael P. Mendenhall, 2018

#ifndef OUTDIRPROCESS_HH
#define OUTDIRPROCESS_HH

#include "ConfigProcess.hh"
#include <TFile.h>

/// Config process associated with a ROOT TFile output directory
class OutDirProcess: public ConfigProcess {
public:
    /// configure directory from parent
    void start_data(DataFrame& F) override;
    /// write/delete output objects and close files at data end
    void end_data(DataFrame& F) override;
protected:
    /// add TObject-derived item to output write list
    template<class T>
    T* addOutput(T* o, const string& n = "") { return (T*)addOutput(dynamic_cast<TObject*>(o), n); }
    vector<pair<string,TObject*>> writeObjs;    ///< items to write and delete at end_data
    TDirectory* myDir = nullptr;                ///< output file directory
};

/// base specialization for saving TObjects to write list
template<>
TObject* OutDirProcess::addOutput(TObject* o, const string& n);

#endif
