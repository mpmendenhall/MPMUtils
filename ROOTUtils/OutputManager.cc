/// \file OutputManager.cc

#include "OutputManager.hh"
#include "PathUtils.hh"
#include "to_str.hh"
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
    printf("\n--------- Building output .root file... ----------\n");
    writeItems(getRootOut());
    if(!parent) {
        delete rootDir;
        rootDir = nullptr;
    }
    printf("---------          Done.          ----------\n");
    return path + ".root";
}

string OutputManager::fullPath() const {
    if(!parent) return path;
    return parent->fullPath() + "/" + path;
}

string OutputManager::printCanvas(const string& fname, const TPad* C, string suffix) const {
    if(!suffix.size()) suffix = printsfx;
    if(squelchAllPrinting) { printf("Printing squelched!\n"); return ""; }

    string fout = fullPath() + "/" + fname + suffix;
    makePath(fout, true);

    if(!C) C = &defaultCanvas;
    if(suffix == ".svgz") {
        string svgout = fullPath() + "/" + fname + ".svg";
        C->Print(svgout.c_str());
        int ret = system(("gzip "+svgout+"; mv "+svgout+".gz "+fout).c_str());
        if(ret) printf("Error gzipping svg -> svgz\n");
    } else {
        C->Print(fout.c_str());
    }

    return fout;
}

