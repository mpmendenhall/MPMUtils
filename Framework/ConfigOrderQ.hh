/// \file ConfigOrderQ.hh Configuration-file-friendly ordering queue
// -- Michael P. Mendenhall, LLNL 2020

#ifndef CONFIGORDERQ_HH
#define CONFIGORDERQ_HH

#include "OrderingQueue.hh"
#include "ConfigFactory.hh"
#include "XMLTag.hh"

/// Re-ordering filter
template<class T>
class ConfigOrderQ: public OrderingQueue<T>, public XMLProvider  {
public:
    /// Constructor
    explicit ConfigOrderQ(const Setting& S): XMLProvider("OrderingQueue") {
        this->createOutput(S["next"]);
        this->dt = 1e9;
        S.lookupValue("dt", this->dt);
    }

protected:
    /// XML output
    void _makeXML(XMLTag& X) override {
        X.addAttr("dt", this->dt);
    }
};

#endif
