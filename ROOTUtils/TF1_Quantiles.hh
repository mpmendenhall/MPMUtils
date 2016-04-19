/// \file TF1_Quantiles.hh Cumulative Density Function and inverse CDF calculator

#include <TArrayD.h>
#include <TF1.h>

/// Quantiles (inverse CDF) distribution from a TF1
/// based on ROOT's TF1::GetQuantiles(...) function
class TF1_Quantiles {
public:
    /// constructor
    TF1_Quantiles(TF1& f);
    /// return quantile for 0 <= p <= 1
    double eval(double p) const;
    /// get average value
    Double_t getAvg() const { return avg; }
    
protected:
    
    const unsigned int npx;
    const Double_t xMin;
    const Double_t xMax;
    const Double_t dx;
    Double_t avg;
    TArrayD integral;
    TArrayD alpha;
    TArrayD beta;
    TArrayD gamma;
};
