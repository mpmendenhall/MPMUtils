/// \file SockDatasink.hh DataSink<> transmitting over socket
// Michael P. Mendenhall, LLNL 2021

#include "SockBinIO.hh"
#include "DataSink.hh"
#include "ConfigFactory.hh"
#include "GlobalArgs.hh"
#include "XMLTag.hh"

/// DataSink<> transmission link over socket connection
template<typename T>
class SockDatasink: public Configurable, public DataSink<T>, public SockBinWrite, public XMLProvider {
public:
    /// Constructor
    explicit SockDatasink(const Setting& S):
    Configurable(S), SockBinWrite("localhost", 50000), XMLProvider("SockDatasink") {
        S.lookupValue("host", host);
        optionalGlobalArg("outhost", host, "data output host");
        S.lookupValue("port", port);
        optionalGlobalArg("outport", port, "data output port");
    }

    /// handle datastream marker signals: always sends data
    void signal(datastream_signal_t s) override {
        if(s == DATASTREAM_INIT) connect_to_socket();
        start_wtx();
        send(vbuff);
        send(s);
        end_wtx();
        vbuff.clear();
        if(s == DATASTREAM_END) finish_mythread();
    }

    void push(T& o) override {
        vbuff.push_back(o);
        if(int(vbuff.size()) >= nvbuff) signal(DATASTREAM_NOOP);
    }

protected:
    int nvbuff = 128;   ///< number of items to group into packet
    vector<typename std::remove_const<T>::type> vbuff;  ///< collect multiple items to buffer
};


/// Receive items for datasink over socket connection
template<typename T>
class SockDSReceiver: public Configurable, public SockConnection, public XMLProvider, public SinkUser<T> {
public:
    /// Constructor
    explicit SockDSReceiver(const Setting& S):
    Configurable(S) {
        S.lookupValue("host", host);
        optionalGlobalArg("inhost", host, "data source host");
        S.lookupValue("port", port);
        optionalGlobalArg("inport", port, "data source port");

        if(S.exists("next")) this->nextSink = constructCfgObj<DataSink<T>>(S["next"]);
        tryAdd(this->nextSink);
    }

    /// Receive data stream
    void run() override {
        if(!this->nextSink) throw std::runtime_error("missing next output");
        create_socket();
        SockBinRead SBR(awaitConnection());

        vector<typename std::remove_const<T>::type> v;
        datastream_signal_t s = DATASTREAM_NOOP;
        while(s != DATASTREAM_END) {
            SBR.receive(v);
            SBR.receive(s);
            for(auto& i: v) this->nextSink->push(i);
            if(s != DATASTREAM_NOOP) this->nextSink->signal(s);
        }
    }
};


