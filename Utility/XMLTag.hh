/// \file XMLTag.hh Simple XML output class
// Michael P. Mendenhall, 2018

#ifndef XMLTAG_HH
#define XMLTAG_HH

#include "TreeWrap.hh"

#include <string>
using std::string;
#include <map>
using std::map;
#include <vector>
using std::vector;
#include <cassert>
#include <iostream>
using std::ostream;
#include <sstream>

/// XML tag base
class _XMLTag {
public:
    /// Constructor
    _XMLTag(const string& nm = ""): name(nm) { }
    /// Destructor
    virtual ~_XMLTag() { }

    /// utility function for converting to string
    template<typename T>
    static string to_str(T x) { std::stringstream ss; ss << x; return ss.str(); }

    /// Add a tag attribute
    virtual void addAttr(const string& nm, const string& val) { attrs[nm] = val; }
    /// Add numerical attribute
    virtual void addAttr(const string& nm, double val) { addAttr(nm, to_str(val)); }
    /// Write output
    virtual void write(ostream& o, unsigned int ndeep = 0, const string& indent = "    ");

    string name;                ///< tag head
    bool oneline = false;       ///< whether to force single-line output
    map<string,string> attrs;   ///< tag attributes

protected:
    /// subclass me! setup before write
    virtual void prepare() { }
    /// generate closing tag
    void closeTag(ostream& o, bool abbrev = false);
};

/// Tree of XML tags
class XMLTag: public TreeWrap<_XMLTag> {
public:
    /// Constructor
    XMLTag(const string& nm = "") { name = nm; }
    /// Write output
    void write(ostream& o, unsigned int ndeep = 0, const string& indent = "    ") override;
};

/// "verbatim contents" XML-includable text
class XMLText: public XMLTag {
public:
    /// Constructor
    XMLText(const string& c): contents(c) { }
    /// write output
    void write(ostream& o, unsigned int ndeep = 0, const string& indent = "    ") override { while(ndeep--) o << indent; o << contents; }
    string contents;    ///< text to include between tags
};

/// Base class for objects that can provide XML output ``on demand''
class _XMLProvider {
public:
    /// Constructor
    _XMLProvider(const string& name): tagname(name) { }
    /// build XML output
    virtual XMLTag* makeXML();
    /// Add a tag attribute
    virtual void addAttr(const string& nm, const string& val) { xattrs[nm] = val; }
    /// Add numerical attribute
    virtual void addAttr(const string& nm, double val) { addAttr(nm, XMLTag::to_str(val)); }

    string tagname;                     ///< this item's tag name

protected:
    /// add class-specific XML data; subclass me!
    virtual void _makeXML(XMLTag&) { }

    map<string,string> xattrs;          ///< tag attributes
};

/// Tree of XML-providing objects
class XMLProvider: public TreeWrap<_XMLProvider> {
public:
    /// Constructor
    using TreeWrap<_XMLProvider>::TreeWrap;
    /// Destructor, releasing ownership of child objects
    virtual ~XMLProvider() { children.clear(); }
    /// build XML output
    XMLTag* makeXML() override;
};

#endif
