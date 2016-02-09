#include "IterRangeFitter.hh"
#include <cassert>
#include <cstdio>

void IterRangeFitter::doFit(TH1* h, const char* opt) {
    assert(myF);
    getRange(fr0, fr1);
    
    int ntries = 0;
    while(ntries++ < nmax) {
        myF->SetRange(fr0,fr1);
        h->Fit(myF, "QR");
        showStep();
        double r0 = fr0, r1 = fr1;
        getRange(fr0,fr1);
        double maxtol = rtol*fabs(fr1-fr0);
        if(fabs(fr0-r0) < maxtol && fabs(fr1-r1) < maxtol) break;
    }
    printf("Iterative fit converged in %i steps.\n", ntries);
    h->Fit(myF, opt);
}

void IterRangeFitter::showStep() const {
    printf("Iterative fit range %g to %g\n", fr0, fr1);
}

IterRangeGaus::IterRangeGaus(double c0, double s0, TF1* f) {
    myF = f? f : new TF1("fGaus","gaus",-1,1);
    myF->SetParameter(1,c0);
    myF->SetParameter(2,s0);
}

void IterRangeGaus::showStep() const {
    printf("Fit step %g +- %g (h=%g)\n", myF->GetParameter(1), myF->GetParameter(2), myF->GetParameter(0));
}

void IterRangeGaus::getRange(double& r0, double& r1) const {
    double c = myF->GetParameter(1);
    double s = fabs(myF->GetParameter(2));
    r0 = c - s*nsigmalo;
    r1 = c + s*nsigmahi;
}

IterRangeErfc::IterRangeErfc(double c0, double s0):
IterRangeGaus(c0, s0, new TF1("iterErfc", "[0]*0.5*(TMath::Erf(-(x-[1])/(sqrt(2)*[2]))+1)", 0, 1)) { }
