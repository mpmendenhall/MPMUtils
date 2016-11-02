/// \file XMLBuilder.cc
// This file was produced under the employ of the United States Government,
// and is consequently in the PUBLIC DOMAIN, free from all provisions of
// US Copyright Law (per USC Title 17, Section 105).
//
// -- Michael P. Mendenhall, 2015

#include "XMLBuilder.hh"

void XMLBuilder::write(ostream& o, unsigned int ndeep, const string& indent) {
    prepare();
    for(unsigned int i=0; i<ndeep; i++) o << indent;
    o << "<" << name;
    for(auto const& kv: attrs) o << " " << kv.first << "=\"" << kv.second << "\"";
    if(children.size()) {
        if(oneline) {
            o << ">";
            for(auto c: children) {
                c->oneline = true;
                c->write(o,0,indent);
            }
        } else {
            o << ">\n";
            for(auto c: children) { c->write(o,ndeep+1,indent); o << "\n"; }
            while(ndeep--) o << indent;
        }
        closeTag(o);
    } else {
        closeTag(o,true);
    }
}

void XMLBuilder::closeTag(ostream& o, bool abbrev) {
    if(abbrev) o << "/>";
    else o << "</" << name << ">";
}

////////////////////////////////

shared_ptr<XMLBuilder> XMLProvider::makeXML() {
    auto B = make_shared<XMLBuilder>(tagname);
    B->attrs = xattrs;
    _makeXML(*B);
    for(auto c: children) B->addChild(c->makeXML());
    return B;
}

