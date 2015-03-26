#ifndef SPLINEFIT_HH
#define SPLINEFIT_HH

#include <TGraphErrors.h>
#include <TF1.h>
#include <TH1.h>
#include <vector>
using std::vector;

/// Provides an interface for TF1 fits of a TGraph cubic spline
class SplineFit {
public:
    /// Constructor
    SplineFit() { }
    /// Destructor
    virtual ~SplineFit() { if(myFitter) delete myFitter; }
    
    /// set spline x positions
    void setX(const double* x, size_t n);
    /// set spline x positions from vector
    void setX(const vector<double>& x) { setX(x.data(), x.size()); }
    /// generate appropriate fitter
    TF1* getFitter();
    /// generate fitter with initial guess for TH1
    TF1* getFitter(const TH1* h);
    /// generate fitter with initial guess for TGraph
    TF1* getFitter(const TGraph* g);
   
    /// update spline to match fit results
    void updateSpline();
    
    
    /// fitter evaluation
    double eval(double* x, double* p);
    
    TGraphErrors mySpline;      ///< fitted spline TGraph with fit errors
    
protected:
    static int nameCounter;     ///< counter for unique naming
    TF1* myFitter = NULL;       ///< fitter for spline
};

#endif

