/// \file OutputManager.cc

#include "OutputManager.hh"
#include "PathUtils.hh"
#include "to_str.hh"
#include "TermColor.hh"
#include <TFile.h>
#include <TStyle.h>

bool OutputManager::squelchAllPrinting = false;

OutputManager::OutputManager(const string& bp, OutputManager* pnt):
parent(pnt), path(bp) {
    TH1::AddDirectory(kFALSE);
    defaultCanvas.SetCanvasSize(200,200);
}

TDirectory* OutputManager::getRootOut() {
    if(!rootDir) {
        if(parent) rootDir = parent->getRootOut()->mkdir(path.c_str());
        else {
            if(!path.size()) throw std::runtime_error("ROOT output path undefined");
            auto fname = fullPath() + ".root";
            makePath(fname, true);
            printf("Writing to '%s'\n", fname.c_str());
            rootDir = new TFile(fname.c_str(), "RECREATE");
        }
    }
    return rootDir;
}

string OutputManager::writeROOT() {
    if(!parent) printf(TERMFG_BLUE "\n--------- Building output .root file... ----------\n");
    writeItems(getRootOut());
    if(!parent) {
        delete rootDir;
        rootDir = nullptr;
    }
    if(!parent) printf(TERMFG_BLUE   "---------   ---  -    Done.   -   ---   ----------" TERMSGR_RESET "\n\n");
    return path + ".root";
}

string OutputManager::fullPath() const {
    if(!parent) return path;
    return parent->fullPath() + "/" + path;
}

void OutputManager::printMulti(const string& fname, const string& suffix, TPad* P) {
    if(!fname.size()) throw std::logic_error("Printset requires non-empty name");

    if(!suffix.size() && printsfx != ".pdf") {
        printf("printMulti '%s' Only supported in .pdf mode\n", fname.c_str());
        printMulti(fname, ".pdf", P);
        return;
    }

    auto& printset = psets[fname];
    if(!printset.n) printset.sfx = suffix;
    else if(printset.sfx != suffix) throw std::logic_error("Inconsistent printset naming");

    printCanvas(fname, P, suffix, printset.n? "" : "(");
    ++printset.n;
}

void OutputManager::endPrintMulti(const string& fname) {
    if(!fname.size()) {
        while(psets.size()) endPrintMulti(psets.begin()->first);
        return;
    }

    auto it = psets.find(fname);
    if(it == psets.end()) return;
    printCanvas(fname, nullptr, it->second.sfx, "]");
    psets.erase(it);
}

string OutputManager::printCanvas(const string& fname, TPad* C, string suffix, const string& xsfx) {
    if(!suffix.size()) suffix = printsfx;
    if(squelchAllPrinting) { printf("Printing squelched!\n"); return ""; }

    string fout = fullPath() + "/" + fname + suffix + xsfx;
    makePath(fout, true);

    if(!C) C = &defaultCanvas;
    if(suffix == ".svgz") {
        string svgout = fullPath() + "/" + fname + ".svg";
        C->Print(svgout.c_str());
        int ret = system(("gzip "+svgout+"; mv "+svgout+".gz "+fout).c_str());
        if(ret) printf("Error gzipping svg -> svgz\n");
    } else {
        C->Print(fout.c_str(), suffix.substr(1, string::npos).c_str());
    }

    return fout;
}

