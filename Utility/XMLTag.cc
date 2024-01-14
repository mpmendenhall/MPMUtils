/// @file XMLTag.cc

#include "XMLTag.hh"

void _XMLTag::write(ostream& o, unsigned int ndeep, const string& indent) {
    prepare();
    for(unsigned int i=0; i<ndeep; i++) o << indent;
    o << "<" << name;
    for(auto const& kv: attrs) o << " " << kv.first << "=\"" << kv.second << "\"";
    closeTag(o,true);
}

void _XMLTag::closeTag(ostream& o, bool abbrev) {
    if(abbrev) o << "/>";
    else o << "</" << name << ">";
}

void XMLTag::write(ostream& o, unsigned int ndeep, const string& indent) {
    prepare();
    for(unsigned int i=0; i<ndeep; i++) o << indent;
    o << "<" << name;
    for(auto const& kv: attrs) o << " " << kv.first << "=\"" << kv.second << "\"";
    if(children.size()) {
        o << ">";
        if(oneline) {
            for(auto c: children) {
                c->oneline = true;
                c->write(o,0,indent);
            }
        } else {
            o << "\n";
            for(auto c: children) { c->write(o,ndeep+1,indent); o << "\n"; }
            while(ndeep--) o << indent;
        }
        closeTag(o);
    } else {
        closeTag(o,true);
    }
}

//////////////////////////////////////////////////

XMLTag* _XMLProvider::makeXML() {
    auto X = new XMLTag(tagname);
    X->attrs = xattrs;
    _makeXML(*X);
    return X;
}

XMLTag* XMLProvider::makeXML() {
    auto X = _XMLProvider::makeXML();
    for(auto c: children) X->addChild(c->makeXML());
    return X;
}
