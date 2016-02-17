/// \file DynamicHistogram.hh histogram classes with dynamically variable bins
#ifndef DYNAMICHISTOGRAM_HH
#define DYNAMICHISTOGRAM_HH

#include <math.h>
#include <map>
using std::map;

/// Dynamic histogram bin contents
class DHBinData {
public:
    /// Constructor
    DHBinData(): wx(0), wxx(0), w(0) { }
    /// constructor with value, weight
    DHBinData(double x, double ww): wx(ww*x), wxx(wx*x), w(ww) { }
    
    /// get mean
    double mu() const { return w? wx/w : 0; }
    /// get mean squared deviation
    double s2() const { return w? (wxx+wx*wx/w)/w : 0; }
    
    /// in-place addition operator
    void operator+=(const DHBinData& r);
    /// < comparison operator
    bool operator<(const DHBinData& r) const { return wx*r.w < r.wx*w; }
    
    double wx;  /// sum w*x
    double wxx; /// sum w*x^2
    double w;   /// sum w
};

/// Dynamically-binned / ``sparse'' histogram base class
class DynamicHistogram {
public:
    /// Constructor
    DynamicHistogram() { }
    /// Destructor
    virtual ~DynamicHistogram() { }

    /// fill new data point
    void fill(double x, double w=1);
    /// get data
    const map<double,DHBinData>& getData() const { return dat; }
    /// get bin with maximum weight
    DHBinData getMax() const;
    
    DHBinData total;            ///< total of all filled data
    
protected:
    /// choose bin center for inserting data point
    virtual double bincenter(const DHBinData& d) const { return d.mu(); }
    /// choose bin for inserting data point
    virtual map<double,DHBinData>::iterator choosebin(const DHBinData& d) = 0;
    
    map<double,DHBinData> dat;  ///< histogram data
};

/// Sparse histogram with fixed-size bins
class SparseHistogram: public DynamicHistogram {
public:
    /// Constructor
    SparseHistogram(double xx0, double ddx): x0(xx0), dx(ddx) {}
    
    const double x0;  ///< one bin center
    const double dx;  ///< bin width
    
protected:
    
    /// choose bin center for inserting data point
    virtual double bincenter(const DHBinData& d) const { return round((d.mu()-x0)/dx); }
    /// choose bin for inserting data point
    virtual map<double,DHBinData>::iterator choosebin(const DHBinData& d) { return dat.find(bincenter(d)); }
};

#endif
