/// \file OutputManager.cc
/*
 * OutputManager.cc, part of the MPMUtils package.
 * Copyright (c) 2014 Michael P. Mendenhall
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */

#include "OutputManager.hh"
#include "PathUtils.hh"
#include <TFile.h>
#include <TDirectory.h>
#include <TH1.h>
#include <TStyle.h>
#include <cassert>

bool OutputManager::squelchAllPrinting = false;

OutputManager::OutputManager(string nm, string bp):
parent(nullptr), name(nm) {
    TH1::AddDirectory(kFALSE);
    // set up output canvas
    defaultCanvas.SetCanvasSize(200,200);
#ifdef PUBLICATION_PLOTS
    defaultCanvas.SetGrayscale(true);
#endif
    plotPath = dataPath = basePath = rootPath = bp;
}

OutputManager::OutputManager(string nm, OutputManager* pnt):
parent(pnt) {
    TH1::AddDirectory(kFALSE);
    // set up output canvas
    defaultCanvas.SetCanvasSize(200,200);
#ifdef PUBLICATION_PLOTS
    defaultCanvas.SetGrayscale(true);
#endif
    rename(nm);
}

void OutputManager::rename(const string& nm) {
    name = nm;
    if(parent) plotPath = dataPath = basePath = rootPath = parent->basePath+"/"+name+"/";
}

TDirectory* OutputManager::writeItems(TDirectory* d) {
    if(!d) d = gDirectory;
    else d = d->mkdir(name.c_str());
    assert(d);
    return TObjCollector::writeItems(d);
}

void OutputManager::writeROOT(TDirectory* parentDir, bool clear) {
    printf("\n--------- Building output .root file... ----------\n");
    TFile* rootOut = nullptr;
    if(parentDir) writeItems(parentDir);
    else {
        makePath(rootPath);
        string outfname = rootPath+"/"+name+".root";
        printf("Writing to '%s'\n", outfname.c_str());
        rootOut = new TFile(outfname.c_str(),"RECREATE");
        rootOut->cd();
        writeItems(nullptr);
    }
    if(clear) clearItems();
    delete rootOut;
    printf("---------          Done.          ----------\n");
}


TH1F* OutputManager::registeredTH1F(string hname, string htitle, unsigned int nbins, float x0, float x1) {
    TH1F* h = new TH1F(hname.c_str(),htitle.c_str(),nbins,x0,x1);
    return (TH1F*)addObject(h);
}

TH1D* OutputManager::registeredTH1D(string hname, string htitle, unsigned int nbins, float x0, float x1) {
    TH1D* h = new TH1D(hname.c_str(),htitle.c_str(),nbins,x0,x1);
    return (TH1D*)addObject(h);
}

TH2F* OutputManager::registeredTH2F(string hname, string htitle, unsigned int nbinsx, float x0, float x1, unsigned int nbinsy, float y0, float y1) {
    return (TH2F*)addObject(new TH2F(hname.c_str(),htitle.c_str(),nbinsx,x0,x1,nbinsy,y0,y1));
}

string OutputManager::printCanvas(const string& fname, const TPad* C, string suffix) const {
    if(!suffix.size()) suffix = printsfx;
    printf("Printing canvas '%s' in '%s'\n",(fname+suffix).c_str(), plotPath.c_str());
    if(squelchAllPrinting) { printf("Printing squelched!\n"); return ""; }

    makePath(plotPath+"/"+fname+suffix,true);
    string fout = plotPath+"/"+fname+suffix;

    if(!C) C = &defaultCanvas;
    if(suffix == ".svgz") {
        string svgout = plotPath+"/"+fname+".svg";
        C->Print(svgout.c_str());
        int ret = system(("gzip "+svgout+"; mv "+svgout+".gz "+fout).c_str());
        if(ret) printf("Error gzipping svg -> svgz\n");
    } else {
        C->Print(fout.c_str());
    }

    return fout;
}

