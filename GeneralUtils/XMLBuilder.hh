/// \file XMLBuilder.hh \brief Simple XML output class
#ifndef XMLBUILDER
#define XMLBUILDER

#include "RefCounter.hh"

#include <string>
using std::string;
#include <map>
using std::map;
#include <iostream>

/// Reference-counted XML tag
class XMLBuilder: public RefCounter {
public:
    /// Constructor
    XMLBuilder(const string& nm = ""): name(nm) { }
    /// Destructor
    virtual ~XMLBuilder() { for(auto it = children.begin(); it != children.end(); it++) (*it)->release(); }
    
    /// Add child node
    virtual void addChild(XMLBuilder* C) { assert(C); C->retain(); children.push_back(C); }
    
    /// Write output
    virtual void write(ostream& o, unsigned int ndeep = 0) {
        prepare();
        for(unsigned int i=0; i<ndeep; i++) o << "\t";
        o << "<" << name;
        for(auto it = attrs.begin(); it != attrs.end(); it++) o << " " << it->first << "=\"" << it->second << "\"";
        if(children.size()) {
            o << ">\n";
            for(auto it = children.begin(); it != children.end(); it++) { (*it)->write(o,ndeep+1); o << "\n"; }
            while(ndeep--) o << "\t";
            closeTag(o);
        } else {
            closeTag(o,true);
        }
    }

    string name;                        ///< tag head
    map<string,string> attrs;           ///< tag attributes
    
protected:
    
    vector<XMLBuilder*> children;       ///< child nodes
    
    /// subclass me! setup before write
    virtual void prepare() { }
    
    virtual void closeTag(ostream& o, bool abbrev = false) {
        if(abbrev) o << "/>";
        else o << "</" << name << ">";
    }
};

/// "verbatim contents" XML-includable text
class XMLText: public XMLBuilder {
public:
    XMLText(const string& c): contents(c) { }
    virtual void write(ostream& o, unsigned int ndeep = 0) { while(ndeep--) o << "\t"; o << contents; }
    string contents;
};

#endif
