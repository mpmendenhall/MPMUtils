/// \file XMLBuilder.hh \brief Simple XML output class

// This file was produced under the employ of the United States Government,
// and is consequently in the PUBLIC DOMAIN, free from all provisions of
// US Copyright Law (per USC Title 17, Section 105).
// 
// -- Michael P. Mendenhall, 2015

#ifndef XMLBUILDER_HH
#define XMLBUILDER_HH

#include "RefCounter.hh"

#include <string>
using std::string;
#include <map>
using std::map;
#include <vector>
using std::vector;
#include <cassert>
#include <iostream>
using std::ostream;

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
    virtual void write(ostream& o, unsigned int ndeep = 0);
    
    string name;                        ///< tag head
    map<string,string> attrs;           ///< tag attributes
    
protected:
    
    vector<XMLBuilder*> children;       ///< child nodes
    
    /// subclass me! setup before write
    virtual void prepare() { }
    /// generate closing tag
    void closeTag(ostream& o, bool abbrev = false);
};

/// "verbatim contents" XML-includable text
class XMLText: public XMLBuilder {
public:
    XMLText(const string& c): contents(c) { }
    virtual void write(ostream& o, unsigned int ndeep = 0) { while(ndeep--) o << "\t"; o << contents; }
    string contents;
};

/// Base class for objects that can provide XML output ``on demand''
class XMLProvider {
public:
    /// Constructor
    XMLProvider(const string& name): tagname(name) { }
    /// Destructor
    virtual ~XMLProvider() { }
    /// build XML output
    XMLBuilder* makeXML();
    /// Add child node
    virtual void addChild(XMLProvider* C) { assert(C); children.push_back(C); }
    
protected:
    /// add class-specific XML data; subclass me!
    virtual void _makeXML(XMLBuilder&) { }
    
    string tagname;                     ///< this item's tag name
    map<string,string> xattrs;          ///< tag attributes
    vector<XMLProvider*> children;      ///< child providers
};

#endif
