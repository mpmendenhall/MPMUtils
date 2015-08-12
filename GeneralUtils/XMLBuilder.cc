/// \file XMLBuilder.cc
// This file was produced under the employ of the United States Government,
// and is consequently in the PUBLIC DOMAIN, free from all provisions of
// US Copyright Law (per USC Title 17, Section 105).
// 
// -- Michael P. Mendenhall, 2015

#include "XMLBuilder.hh"

string XMLBuilder::indent = "    ";

void XMLBuilder::write(ostream& o, unsigned int ndeep) {
    prepare();
    for(unsigned int i=0; i<ndeep; i++) o << indent;
    o << "<" << name;
    for(auto it = attrs.begin(); it != attrs.end(); it++) o << " " << it->first << "=\"" << it->second << "\"";
    if(children.size()) {
        if(oneline) {
            o << ">";
            for(auto it = children.begin(); it != children.end(); it++) {
                (*it)->oneline = true;
                (*it)->write(o,0);
            }
        } else {
            o << ">\n";
            for(auto it = children.begin(); it != children.end(); it++) { (*it)->write(o,ndeep+1); o << "\n"; }
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

XMLBuilder* XMLProvider::makeXML() {
    XMLBuilder* B = new XMLBuilder(tagname);
    B->attrs = xattrs;
    _makeXML(*B);
    for(auto it = children.begin(); it != children.end(); it++) B->addChild((*it)->makeXML());
    return B;
}

