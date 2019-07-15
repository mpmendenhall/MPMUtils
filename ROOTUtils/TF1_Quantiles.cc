#include "TF1_Quantiles.hh"
#include <stdexcept>
#include <TMath.h>

TF1_Quantiles::TF1_Quantiles(TF1& f): npx(f.GetNpx()), xMin(f.GetXmin()), xMax(f.GetXmax()), dx((xMax-xMin)/npx),
integral(npx+1), alpha(npx), beta(npx), gamma(npx) {

    assert(npx);

    integral[0] = 0;
    Int_t intNegative = 0;
    avg = 0;
    for (unsigned int i = 0; i < npx; i++) {
        Double_t integ = f.Integral(xMin+i*dx, xMin+i*dx+dx);
        if (integ < 0) {intNegative++; integ = -integ;}
        integral[i+1] = integral[i] + integ;
        avg += integ * (xMin+(i+0.5)*dx);
    }

    const Double_t total = integral[npx];
    avg /= total;

    if(intNegative > 0 || !total) throw std::runtime_error("bad probability distribution");

    // normalize integral to CDF
    for (unsigned int i = 1; i <= npx; i++) integral[i] /= total;

    //the integral r for each bin is approximated by a parabola
    //  x = alpha + beta*r +gamma*r**2
    // compute the coefficients alpha, beta, gamma for each bin
    for (unsigned int i = 0; i < npx; i++) {
        const Double_t x0 = xMin+dx*i;
        const Double_t r2 = integral[i+1]-integral[i];
        const Double_t r1 = f.Integral(x0,x0+0.5*dx)/total;
        gamma[i] = (2*r2-4*r1)/(dx*dx);
        beta[i]  = r2/dx-gamma[i]*dx;
        alpha[i] = x0;
        gamma[i] *= 2;
    }
}

double TF1_Quantiles::eval(double p) const {

    UInt_t bin  = TMath::Max(TMath::BinarySearch(npx+1,integral.GetArray(),p),(Long64_t)0);
    // LM use a tolerance 1.E-12 (integral precision)
    while (bin < npx-1 && TMath::AreEqualRel(integral[bin+1], p, 1E-12) ) {
        if (TMath::AreEqualRel(integral[bin+2], p, 1E-12) ) bin++;
        else break;
    }

    const Double_t rr = p-integral[bin];
    Double_t x;
    if (rr != 0.0) {
        Double_t xx = 0.0;
        const Double_t fac = -2.*gamma[bin]*rr/beta[bin]/beta[bin];
        if (fac != 0 && fac <= 1)
            xx = (-beta[bin]+TMath::Sqrt(beta[bin]*beta[bin]+2*gamma[bin]*rr))/gamma[bin];
        else if (beta[bin] != 0.)
            xx = rr/beta[bin];
        x = alpha[bin]+xx;
    } else {
        x = alpha[bin];
        if (integral[bin+1] == p) x += dx;
    }

    return x;
}
