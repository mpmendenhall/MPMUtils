/// @file HDF5_CfgLoader.hh Base for configurable HDF5 data table input/output

#ifndef HDF5_CFGLOADER_HH
#define HDF5_CFGLOADER_HH

#include "CfgLoader.hh"
#include "HDF5_Table_Cache.hh"

/// Scan generic data from HDF5 file
template<typename T>
class HDF5_CfgLoader: public HDF5_Table_Cache<T>, public CfgLoader<T> {
public:
    /// Constructor
    explicit HDF5_CfgLoader(const Setting& S, const string& farg = "", const string& tname = "", int v = 0):
    XMLProvider("HDF5_CfgLoader"), HDF5_Table_Cache<T>(tname, v), CfgLoader<T>(S, farg) { }
};

/// Write generic data to HDF5 file
template<typename T>
class HDF5_CfgWriter: public HDF5_Table_Writer<T>, virtual public XMLProvider {
public:
    /// Constructor
    explicit HDF5_CfgWriter(const Setting&, const string& farg = ""): XMLProvider("HDF5_CfgWriter") {
        if(!farg.size()) return;
        const auto& fn = requiredGlobalArg(farg, "output .h5 file");
        this->openOutput(fn);

        auto AS = AnalysisStep::instance();
        if(AS) AS->outfilename = fn;
    }

protected:
    /// build XML output data
    void _makeXML(XMLTag& X) override {
        X.addAttr("nWritten", this->getNWrite());
    }
};

#endif
