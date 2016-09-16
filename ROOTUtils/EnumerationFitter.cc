/// \file EnumerationFitter.cc
/*
 * EnumerationFitter.hh, part of the MPMUtils package.
 * Copyright (c) 2007-2014 Michael P. Mendenhall
 *
 * This code uses the LAPACKE C interface to LAPACK;
 * see http://www.netlib.org/lapack/lapacke.html
 * and the GSL interface to CBLAS, https://www.gnu.org/software/gsl/
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

#include "EnumerationFitter.hh"
#include "SMExcept.hh"
#include "StringManip.hh"
#include "PathUtils.hh"



double EnumerationFitter::Evaluate(double *x, double *p) {
    smassert(x);
    int i = (int)(*x);
    double s = 0;
    for(unsigned int n=0; n<fterms.size(); n++) {
        if(i>=0 && i<(int)fterms[n].size()) s += fterms[n][i]*p[n];
    }
    return s;
}

void EnumerationFitter::addTerm(const vector<double>& t) {
    if(fitter) {
        delete fitter;
        fitter = nullptr;
    }
    fterms.push_back(t);
}

TF1* EnumerationFitter::getFitter() {
    if(!fitter)
        fitter = new TF1("fEnumFit",this,&EnumerationFitter::Evaluate,0,1,fterms.size());
    return fitter;
}

TGraphErrors* EnumerationFitter::loadFitFile(const string& fname) {
    if(!fileExists(fname)) {
        SMExcept e("fileUnreadable");
        e.insert("filename",fname);
        throw(e);
    }

    std::ifstream fin(fname.c_str());
    string s;
    vector<double> datenum;
    vector<double> dat;
    vector<double> daterr;
    fterms.clear();
    if(fitter) {
        delete fitter;
        fitter = nullptr;
    }

    printf("Loading data from '%s'...\n",fname.c_str());
    while (fin.good()) {
        std::getline(fin,s);
        s = strip(s);
        if(!s.size() || s[0]=='#')
            continue;
        vector<double> v = sToDoubles(s," ,\t");
        if(v.size() < 2) continue;
        datenum.push_back(0.5+dat.size());
        dat.push_back(v[0]);
        daterr.push_back(v[1]);
        for(unsigned int i=2; i<v.size(); i++) {
            while(fterms.size()<i-1)
                fterms.push_back(vector<double>());
            fterms[i-2].push_back(v[i]);
        }
    }
    fin.close();

    printf("Loaded %i fit points and %i parameters\n",(int)dat.size(),(int)fterms.size());

    return new TGraphErrors(dat.size(),&datenum[0],&dat[0],nullptr,&daterr[0]);
}
