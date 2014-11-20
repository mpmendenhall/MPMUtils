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
#include <TH1.h>

bool OutputManager::squelchAllPrinting = false;

OutputManager::OutputManager(string nm, string bp): rootOut(NULL), defaultCanvas(new TCanvas()),
parent(NULL), writeRootOnDestruct(false) {
    TH1::AddDirectory(kFALSE);
    // set up output canvas
    defaultCanvas->SetFillColor(0);
    defaultCanvas->SetCanvasSize(300,300);
    #ifdef PUBLICATION_PLOTS
    defaultCanvas->SetGrayscale(true);
    #endif
    
    basePath = plotPath = dataPath = rootPath = bp;
    setName(nm);
}

OutputManager::OutputManager(string nm, OutputManager* pnt):
rootOut(NULL), defaultCanvas(NULL), parent(pnt), writeRootOnDestruct(false) {
    TH1::AddDirectory(kFALSE);
    if(parent)
        defaultCanvas = parent->defaultCanvas;
    setName(nm);
}

void OutputManager::setName(string nm) {
    name = nm;
    if(parent) {
        plotPath = dataPath = basePath = rootPath = parent->basePath+"/"+name+"/";
    }
}

void OutputManager::warn(WarningLevel l, string descrip, Stringmap M) {
    
    M.insert("description",descrip);
    M.insert("subsystem",name);
    
    if(l==BENIGN_WARNING) {
        printf("* Warning: %s\n",descrip.c_str());
        M.insert("level","benign");
    } else if(l==MODERATE_WARNING) {
        printf("\n*** WARNING: %s\n",descrip.c_str());
        M.insert("level","moderate");
    } else if(l==SEVERE_WARNING) {
        printf("\n\n*******************\n* SEVERE WARNING: %s\n*******************\n",descrip.c_str());
        M.insert("level","severe");
    } if(l==FATAL_WARNING) {
        printf("\n\n*******************\n*******************\n** FATAL WARNING: %s\n*******************\n*******************\n",descrip.c_str());
        M.insert("level","fatal");
    }
    
    M.display("\t");
    printf("\n");
    
    if(parent)
        parent->qOut.insert("Warning",M);
    else
        qOut.insert("Warning",M);
}

void OutputManager::write(string outName) {
    // write text data file
    if(qOut.size()) {
        makePath(dataPath+"/"+outName,true);
        if(outName.size())
            qOut.setOutfile(dataPath+"/"+outName);
        else
            qOut.setOutfile(dataPath+"/"+name+".txt");
        qOut.commit();
    }
}

void OutputManager::openOutfile() {
    if(rootOut) { rootOut->Close(); }
    makePath(rootPath);
    rootOut = new TFile((rootPath+"/"+name+".root").c_str(),"RECREATE");
    rootOut->cd();
}

void OutputManager::writeROOT() {
    printf("\n--------- Building output .root file... ----------\n");
    if(!rootOut) openOutfile();
    rootOut->cd();
    writeItems();
    clearItems();
    rootOut->Close();
    rootOut = NULL;
    printf("---------          Done.          ----------\n");
}


TH1F* OutputManager::registeredTH1F(string hname, string htitle, unsigned int nbins, float x0, float x1) {
    if(rootOut) rootOut->cd();
    return (TH1F*)addObject(new TH1F(hname.c_str(),htitle.c_str(),nbins,x0,x1));
}

TH2F* OutputManager::registeredTH2F(string hname, string htitle, unsigned int nbinsx, float x0, float x1, unsigned int nbinsy, float y0, float y1) {
    if(rootOut) rootOut->cd();
    return (TH2F*)addObject(new TH2F(hname.c_str(),htitle.c_str(),nbinsx,x0,x1,nbinsy,y0,y1));
}

void OutputManager::printCanvas(string fname, string suffix) const {
    printf("Printing canvas '%s' in '%s'\n",(fname+suffix).c_str(), plotPath.c_str());
    if(squelchAllPrinting) { printf("Printing squelched!\n"); return; }
    makePath(plotPath+"/"+fname+suffix,true);
    defaultCanvas->Print((plotPath+"/"+fname+suffix).c_str());
}

