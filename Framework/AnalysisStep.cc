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
#include "TermColor.hh"
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
codename(cd), t0(time(nullptr)), pt0(steady_clock::now()),
anatag(PROJ_ENV_PFX()+"-Analysis") { }

bool AnalysisStep::make_xmlout() {
    if(!outfilename.size()) {
        printf(TERMFG_YELLOW "\nNo file specified for .xml output.\n\n" TERMFG_GREEN);
        auto X = makeXML();
        X->write(std::cout);
        printf(TERMSGR_RESET "\n\n");
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
    if(!prevdat.size()) printf(TERMFG_YELLOW "No previous xml metadata found!\n");

    printf(TERMFG_GREEN "Writing .xml metadata to '%s.xml'" TERMSGR_RESET "\n", outfilename.c_str());
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
    X.addAttr("start_time", t0);
    X.addAttr("running_time", std::chrono::duration<double>(steady_clock::now()-pt0).count());
    X.attrs["host"] = CodeVersion::host;
    X.attrs["user"] = CodeVersion::user;

    for(auto const& f: infiles)  X.addChild(infileEntry(f));
    auto Xfout = new XMLTag("output");
    Xfout->attrs["file"] = outfilename;
    X.addChild(Xfout);
}
