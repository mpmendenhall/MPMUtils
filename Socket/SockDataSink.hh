/// \file SockDataSink.hh DataSink<> transmitting over socket
// Michael P. Mendenhall, LLNL 2021

#include "SockBinIO.hh"
#include "DataSink.hh"
#include "ConfigThreader.hh"
#include "GlobalArgs.hh"
#include "XMLTag.hh"

/// DataSink<> transmission link over socket connection
template<typename T>
class SockDataSink: public Configurable, public DataSink<T>, public SockBinWrite, public XMLProvider {
public:
    /// Constructor
    explicit SockDataSink(const Setting& S):
    Configurable(S), SockBinWrite("localhost", 50000), XMLProvider("SockDataSink") {
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

/// Base class configurable multithreaded socket server
class ConfigSockServer: public ConfigThreader, public SockConnection {
public:
    /// Constructor
    explicit ConfigSockServer(const Setting& S):
    XMLProvider("ConfigSockServer"), ConfigThreader(S, -2) {
        S.lookupValue("host", host);
        optionalGlobalArg("inhost", host, "data source host");
        S.lookupValue("port", port);
        optionalGlobalArg("inport", port, "data source port");
    }

    /// Destructor
    ~ConfigSockServer() { for(auto kv: myCTs) delete kv.second; }

    /// Receive data stream
    void run() override {
        create_socket();
        launch_mythread();

        while(runstat != STOP_REQUESTED) {
            int c = awaitConnection();
            lock_guard<mutex> l(inputMut);
            //myCTs[c] = constructCfgObj<ConfigThreader>(Cfg["thread"], c);
            myCTs[c]->launch_mythread();
        }

        for(auto kv: myCTs) kv.second->finish_mythread();
    }

protected:
    map<int, ConfigThreader*> myCTs;
};

/// Receive items for datasink over socket connection, in format vector(items, ...),signal
template<typename T>
class SockDSVecReceiver: public ConfigSockServer, public SinkUser<T> {
public:
    /// Constructor
    explicit SockDSVecReceiver(const Setting& S): XMLProvider("SockDSVecReceiver"),
    ConfigSockServer(S) { if(S.exists("next")) this->createOutput(S["next"]); }

    using SinkUser<T>::nextSink;

    /// Receive data stream
    void run() override {
        if(!nextSink) throw std::runtime_error("missing next output");
        create_socket();
        SockBinRead SBR(awaitConnection());

        vector<typename std::remove_const<T>::type> v;
        datastream_signal_t s = DATASTREAM_NOOP;
        while(s != DATASTREAM_END) {
            SBR.receive(v);
            SBR.receive(s);
            for(auto& i: v) this->nextSink->push(i);
            if(s != DATASTREAM_NOOP) nextSink->signal(s);
        }
    }
};

/// Receive items for DataSink over socket
template<typename T>
class SockDSReceiver: public ConfigSockServer, public SinkUser<T> {
public:
    /// Constructor
    explicit SockDSReceiver(const Setting& S):
    ConfigSockServer(S) { if(S.exists("next")) this->createOutput(S["next"]); }

    using SinkUser<T>::nextSink;

    // Receive data stream
    void run() override {
        if(!nextSink) throw std::runtime_error("missing next output");

        create_socket();
        printf("Awaiting data connection on '%s:%i'\n", host.c_str(), port);
        SockBinRead SBR(awaitConnection());
        printf("Receiving data on '%s:%i'\n", host.c_str(), port);

        nextSink->signal(DATASTREAM_INIT);

        typename SinkUser<T>::outmut_t o;
        while(true) {
            try { SBR.receive(o); }
            catch(SockFDerror& e) {
                printf("Ending socket input on '%s'\n", e.what());
                break;
            }
            nextSink->push(o);
        }

        nextSink->signal(DATASTREAM_FLUSH);
        nextSink->signal(DATASTREAM_END);
    }
};

/// Opaque blob receiver
class SockDSBlobReceiver: public ConfigSockServer,
public SinkUser<const vector<char>> {
public:
    /// Constructor
    explicit SockDSBlobReceiver(const Setting& S):
    ConfigSockServer(S) { if(S.exists("next")) this->createOutput(S["next"]); }

    // Receive data stream
    void run() override {
        if(!nextSink) throw std::runtime_error("missing next output");

        create_socket();
        printf("Awaiting data connection on '%s:%i'\n", host.c_str(), port);
        SockFD S(awaitConnection());
        printf("Got connection descriptor %i\n", S.sockfd);

        nextSink->signal(DATASTREAM_INIT);

        vector<char> v;
        while(true) {
            S.vecread(v);
            if(v.size()) nextSink->push(v);
            else break;
        }

        nextSink->signal(DATASTREAM_FLUSH);
        nextSink->signal(DATASTREAM_END);
    }
};
