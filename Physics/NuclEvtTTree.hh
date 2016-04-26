/// \file NuclEvtTTree.hh Interface for loading events from TTree

#include "NuclEvtGen.hh"
#include "TChainScanner.hh"

/// class for reading stored events trees
class EventTreeScanner: protected TChainScanner {
public:
    /// constructor
    EventTreeScanner(): TChainScanner("Evts") {}
    /// add a file to the TChain
    virtual int addFile(const std::string& filename);
    /// load next event into vector; return number of primaries
    unsigned int loadEvt(vector<NucDecayEvent>& v);
    
    bool firstpass;     ///< whether read is on first pass through data
    
protected:
    /// set tree readpoints
    virtual void setReadpoints(TTree* T);
    
    NucDecayEvent evt;  ///< event readpoint
    unsigned int prevN; ///< previous event number
};

/// base class for generating event positions
class PositionGenerator {
public:
    /// constructor
    PositionGenerator() {}
    /// destructor
    virtual ~PositionGenerator() {}
    
    /// get number of random DF consumed
    virtual unsigned int getNDF() const { return 3; }
    /// generate vertex position
    virtual void genPos(double* v, double* rnd = nullptr) const = 0;
};

/// map unit square onto circle of specified radius
void square2circle(double& x, double& y, double r = 1.0);

/// class for generating positions in a cylinder
class CylPosGen: public PositionGenerator {
public:
    /// constructor
    CylPosGen(double zlength, double radius): dz(zlength), r(radius) {}
    /// generate vertex position
    virtual void genPos(double* v, double* rnd = nullptr) const;
    
    double dz;  ///< length of cylinder
    double r;   ///< radius of cylinder
};

/// uniform cube [0,1]^3 positions, for later transform
class CubePosGen: public PositionGenerator {
public:
    /// constructor
    CubePosGen() {}
    /// generate vertex position
    virtual void genPos(double* v, double* rnd = nullptr) const;
};

/// generate fixed event position
class FixedPosGen: public PositionGenerator {
public:
    /// constructor
    FixedPosGen(double x0=0, double y0=0, double z0=0): x(x0), y(y0), z(z0) {}
    /// generate vertex position
    virtual void genPos(double* v, double* = nullptr) const { v[0]=x; v[1]=y; v[2]=z; }
    /// get number of random DF consumed
    virtual unsigned int getNDF() const { return 0; }
    
    double x;
    double y;
    double z;
};
