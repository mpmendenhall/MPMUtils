/// \file AnalysisStep.cc
// This file was produced under the employ of the United States Government,
// and is consequently in the PUBLIC DOMAIN, free from all provisions of
// US Copyright Law (per USC Title 17, Section 105).
//
// -- Michael P. Mendenhall, 2015

#include "AnalysisStep.hh"
#include "PathUtils.hh"
#include "GetEnv.hh"
#include "StringManip.hh"
#include <fstream>
#include <stdio.h>

string md5sum(const string& f) {
    FILE *p = popen(("md5sum "+f).c_str(), "r");
    if (p == nullptr) return "";
    string md5;
    char c;
    for(int i = 0; i < 32 && isxdigit(c = fgetc(p)); i++) md5 += c;
    pclose(p);
    return md5;
}

AnalysisStep::AnalysisStep(const string& cd): XMLProvider("AnalysisStep"),
codename(cd), t0(time(nullptr)), anatag(PROJ_ENV_PFX()+"-Analysis") { }

bool AnalysisStep::make_xmlout() {
    if(!outfilename.size()) {
        printf("No file specified for .xml output.\n\n");
        auto X = makeXML();
        X->write(std::cout);
        printf("\n\n");
        delete X;
        return false;
    }

    string xmlin = "";
    for(auto const& f: infiles) {
        xmlin = split(f,".").back()=="xml"? f : f+".xml";
        if(fileExists(xmlin)) break;
        xmlin = "";
    }

    makePath(outfilename, true);

    // read previous data (unparsed)
    string prevdat = "";
    if(xmlin.size()) {
        std::ifstream oldf(xmlin);
        string line;
        bool append = false;
        while (std::getline(oldf, line)) {
            string sline = strip(line);
            if(sline == "<"+anatag+">") { append = true; continue; }
            if(sline == "</"+anatag+">") break;
            if(append) prevdat += line + "\n";
        }
    }
    if(!prevdat.size()) printf("No previous xml metadata found!\n");

    printf("Writing .xml metadata to '%s.xml'\n", outfilename.c_str());
    std::ofstream o(outfilename+".xml");
    o << "<?xml version=\"1.0\"?>\n";
    o << "<" << anatag << ">\n";
    o << prevdat;
    auto X = makeXML();
    X->write(o,1);
    o << "\n</" << anatag << ">\n";

    return true;
}

XMLTag* infileEntry(const string& f) {
    auto Xfin = new XMLTag("input");
    Xfin->attrs["file"] = f;
    Xfin->attrs["md5"] = md5sum(f);
    return Xfin;
}

void AnalysisStep::_makeXML(XMLTag& X) {
    X.attrs["code"] = codename;
    X.attrs["git_hash"] = CodeVersion::repo_version;
    X.attrs["git_tag"] = CodeVersion::repo_tagname;
    X.attrs["compiler"] = CodeVersion::compiler;
    X.addAttr("time", time(nullptr));
    X.addAttr("dtime", time(nullptr)-t0);
    X.attrs["host"] = CodeVersion::host;
    X.attrs["user"] = CodeVersion::user;

    for(auto const& f: infiles)  X.addChild(infileEntry(f));
    auto Xfout = new XMLTag("output");
    Xfout->attrs["file"] = outfilename;
    X.addChild(Xfout);
}
