/// \file XMLBuilder.hh Simple XML output class

// This file was produced under the employ of the United States Government,
// and is consequently in the PUBLIC DOMAIN, free from all provisions of
// US Copyright Law (per USC Title 17, Section 105).
//
// -- Michael P. Mendenhall, 2015

#ifndef XMLBUILDER_HH
#define XMLBUILDER_HH

#include "StringManip.hh"

#include <memory>
using std::shared_ptr;
using std::make_shared;
#include <map>
using std::map;
#include <vector>
using std::vector;
#include <cassert>
#include <iostream>
using std::ostream;

/// Reference-counted XML tag
class XMLBuilder {
public:
    /// Constructor
    XMLBuilder(const string& nm = ""): name(nm) { }
    /// Destructor
    virtual ~XMLBuilder() { }

    /// Add child node
    virtual void addChild(const shared_ptr<XMLBuilder>& C) { children.push_back(C); }
    /// Add a tag attribute
    virtual void addAttr(const string& nm, const string& val) { attrs[nm] = val; }
    /// Add numerical attribute
    virtual void addAttr(const string& nm, double val) { addAttr(nm, to_str(val)); }

    /// Write output
    virtual void write(ostream& o, unsigned int ndeep = 0, const string& indent = "    ");

    string name;                        ///< tag head
    bool oneline = false;               ///< whether to force single-line output
    map<string,string> attrs;           ///< tag attributes

protected:

    vector<shared_ptr<XMLBuilder>> children;       ///< child nodes

    /// subclass me! setup before write
    virtual void prepare() { }
    /// generate closing tag
    void closeTag(ostream& o, bool abbrev = false);
};

/// "verbatim contents" XML-includable text
class XMLText: public XMLBuilder {
public:
    /// Constructor
    XMLText(const string& c): contents(c) { }
    /// write output
    void write(ostream& o, unsigned int ndeep = 0, const string& indent = "    ") override { while(ndeep--) o << indent; o << contents; }
    string contents;    ///< text to include between tags
};

/// Base class for objects that can provide XML output ``on demand''
class XMLProvider {
public:
    /// Constructor
    XMLProvider(const string& name): tagname(name) { }
    /// Destructor
    virtual ~XMLProvider() { }
    /// build XML output
    shared_ptr<XMLBuilder> makeXML();
    /// Add child node
    virtual void addChild(const shared_ptr<XMLProvider>& C) { children.push_back(C); }
    /// Add externally-memory-managed child node
    virtual void addChild(XMLProvider* C) { addChild(shared_ptr<XMLProvider>(shared_ptr<XMLProvider>(), C)); }
    /// Add a tag attribute
    virtual void addAttr(const string& nm, const string& val) { xattrs[nm] = val; }
    /// Add numerical attribute
    virtual void addAttr(const string& nm, double val) { addAttr(nm, to_str(val)); }

    string tagname;                     ///< this item's tag name

protected:
    /// add class-specific XML data; subclass me!
    virtual void _makeXML(XMLBuilder&) { }

    map<string,string> xattrs;          ///< tag attributes
    vector<shared_ptr<XMLProvider>> children;      ///< child providers
};

#endif
