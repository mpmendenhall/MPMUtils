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
class HDF5_CfgLoader: public Configurable, public HDF5_TableInput<T>, virtual public XMLProvider, public SinkUser<const T> {
public:
    using SinkUser<const T>::nextSink;
    using HDF5_TableInput<T>::nLoad;
    using HDF5_TableInput<T>::id_current_evt;
    using HDF5_TableInput<T>::getIdentifier;

    /// Constructor
    explicit HDF5_CfgLoader(const Setting& S, const string& farg = "", bool doMakeNext = true, const string& tname = "", int v = 0):
    XMLProvider("HDF5_CfgLoader"), Configurable(S), HDF5_TableInput<T>(tname, v) {
        S.lookupValue("nLoad", nLoad);
        optionalGlobalArg("nload", nLoad, "entry loading limit");
        S.lookupValue("eventwise", eventwise);

        if(farg.size()){
            auto& fn = requiredGlobalArg(farg);
            this->openInput(fn);
        }

        if(doMakeNext) makeNext(S);
    }

    /// Push input file contents to nextSink
    void run() override {
        if(!nextSink) throw std::runtime_error("HDF5 scanner 'next' output not configured.");
        if(!this->infile_id) throw std::runtime_error("HDF5 scanner run without opening input file.");

        auto AS = AnalysisStep::instance();
        if(AS) AS->infiles.push_back(this->infile_name);

        nextSink->signal(DATASTREAM_INIT);

        auto fRows = this->getNRows();
        if(nLoad >= 0 && hsize_t(nLoad) < fRows) fRows = nLoad;
        {
            T P;
            ProgressBar PB(fRows);
            while(this->next(P) && !++PB) {
                if(eventwise) {
                    auto idP = getIdentifier(P);
                    if(idP != id_current_evt) {
                        nextSink->signal(DATASTREAM_FLUSH);
                        id_current_evt = idP;
                    }
                }
                nextSink->push(P);
            }
        }

        nextSink->signal(DATASTREAM_FLUSH);
        nextSink->signal(DATASTREAM_END);
    }

    bool eventwise = false; ///< whether to flush on event number changes

protected:
    /// configure nextSink
    void makeNext(const Setting& S) {
        if(S.exists("next")) this->createOutput(S["next"]);
        else {
            string nxt;
            if(optionalGlobalArg("h5next", nxt, "HDF5 reader next output class"))
                nextSink = BaseFactory<DataSink<const T>>::construct(nxt);
            tryAdd(nextSink);
        }
    }

    /// build XML output data
    void _makeXML(XMLTag& X) override {
        X.addAttr("nRows", this->getNRows());
        if(nLoad >= 0) X.addAttr("nLoad", nLoad);
        if(eventwise) X.addAttr("eventwise", "true");
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
