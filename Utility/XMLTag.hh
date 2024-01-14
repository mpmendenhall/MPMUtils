/// @file XMLTag.hh Simple XML output class
// -- Michael P. Mendenhall, 2018

#ifndef XMLTAG_HH
#define XMLTAG_HH

#include "TreeWrap.hh"
#include "to_str.hh"

#include <map>
using std::map;
#include <iostream>
using std::ostream;

/// XML tag base
class _XMLTag {
public:
    /// Constructor
    explicit _XMLTag(const string& _name = ""): name(_name) { }
    /// Destructor
    virtual ~_XMLTag() { }

    /// Add a tag attribute
    virtual void addAttr(const string& attrnm, const string& val) { attrs[attrnm] = val; }
    /// Add a tag attribute
    template<typename C>
    void addAttr(const string& attrnm, const C& val) { addAttr(attrnm, to_str(val)); }
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
    explicit XMLTag(const string& _name = "") { name = _name; }
    /// Write output
    void write(ostream& o, unsigned int ndeep = 0, const string& indent = "    ") override;
};

/// "verbatim contents" XML-includable text
class XMLText: public XMLTag {
public:
    /// Constructor
    explicit XMLText(const string& c): contents(c) { }
    /// write output
    void write(ostream& o, unsigned int ndeep = 0, const string& indent = "    ") override { while(ndeep--) o << indent; o << contents; }
    string contents;    ///< text to include between tags
};

/// Base class for objects that can provide XML output ``on demand''
class _XMLProvider {
public:
    /// Constructor
    explicit _XMLProvider(const string& name = "UNKNOWN"): tagname(name) { }
    /// build XML output
    virtual XMLTag* makeXML();
    /// Add a tag attribute
    virtual void addAttr(const string& attrnm, const string& val) { xattrs[attrnm] = val; }
    /// Add a tag attribute
    template<typename C>
    void addAttr(const string& attrnm, const C& val) { addAttr(attrnm, to_str(val)); }

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
