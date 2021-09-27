/// \file HDF5_CfgLoader.hh Base for configurable HDF5 data table input/output

#ifndef HDF5_CFGLOADER_HH
#define HDF5_CFGLOADER_HH

#include "HDF5_IO.hh"
#include "GlobalArgs.hh"
#include "ConfigFactory.hh"
#include "AnalysisStep.hh"
#include "ProgressBar.hh"

/// Scan generic data from HDF5 file
template<typename T>
class HDF5_CfgLoader: public Configurable, public HDF5_TableInput<T>, virtual public XMLProvider {
public:
    /// Constructor
    explicit HDF5_CfgLoader(const Setting& S, const string& farg = "", bool doMakeNext = true, const string& tname = "", int v = 0):
    XMLProvider("HDF5_CfgLoader"), Configurable(S), HDF5_TableInput<T>(tname, v) {
        S.lookupValue("nLoad", nLoad);
        optionalGlobalArg("nload", nLoad, "entry loading limit");

        if(farg.size()){
            auto& fn = requiredGlobalArg(farg);
            this->openInput(fn);
        }

        if(doMakeNext) makeNext(S);
    }

    /// Destructor
    ~HDF5_CfgLoader() { delete nextSink; }

    /// Push input file contents to nextSink
    void run() override {
        if(!nextSink) throw std::runtime_error("HDF5 scanner 'next' output not configured.");
        if(!this->infile_id) throw std::runtime_error("HDF5 scanner run without opening input file.");

        auto AS = AnalysisStep::instance();
        if(AS) AS->infiles.push_back(this->infile_name);

        fRows = this->getNRows();
        {
            T P;
            ProgressBar PB(nLoad >= 0? nLoad : fRows);
            nextSink->signal(DATASTREAM_INIT);
            while(this->next(P) && !++PB) { nextSink->push(P); }
        }
        nextSink->signal(DATASTREAM_FLUSH);
        nextSink->signal(DATASTREAM_END);
    }

    DataSink<const T>* nextSink = nullptr;  ///< next step in chain --- owned by this
    int nLoad = -1;                     ///< maximum events to load
    hsize_t fRows = 0;                  ///< number of rows in input file

protected:
    /// configure nextSink
    void makeNext(const Setting& S) {
        if(S.exists("next")) nextSink = constructCfgObj<DataSink<const T>>(S["next"]);
        else {
            string nxt;
            if(optionalGlobalArg("h5next", nxt, "HDF5 reader next output class"))
                nextSink = constructCfgClass<DataSink<const T>>(nxt);
        }
        tryAdd(nextSink);
    }
    /// build XML output data
    void _makeXML(XMLTag& X) override {
        X.addAttr("nRows", fRows);
        if(nLoad >= 0) X.addAttr("nLoad", nLoad);
    }
};

/// Write generic data to HDF5 file
template<typename T>
class HDF5_CfgWriter: public HDF5_TableOutput<T>, virtual public XMLProvider {
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
