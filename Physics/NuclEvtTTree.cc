#include "NuclEvtTTree.hh"
#include "AxisEnum.hh"
#include <TRandom.h>

void EventTreeScanner::setReadpoints(TTree* T) {
    SetBranchAddress(T,"num",&evt.eid);
    SetBranchAddress(T,"PID",&evt.d);
    SetBranchAddress(T,"KE",&evt.E);
    SetBranchAddress(T,"vertex",evt.x);
    SetBranchAddress(T,"direction",evt.p);
    SetBranchAddress(T,"time",&evt.t);
    SetBranchAddress(T,"weight",&evt.w);
}

int EventTreeScanner::addFile(const std::string& filename) {
    int nf = TChainScanner::addFile(filename);
    startScan();
    nextPoint();
    prevN = evt.eid;
    firstpass = true;
    return nf;
}

unsigned int EventTreeScanner::loadEvt(vector<NucDecayEvent>& v) {
    unsigned int nevts = 0;
    do {
        v.push_back(evt);
        ++nevts;
        nextPoint();
    } while(prevN==evt.eid);
    firstpass &= evt.eid>=prevN;
    prevN=evt.eid;
    return nevts;
}

//-----------------------------------------

void square2circle(double& x, double& y, double r) {
    double th = 2*M_PI*x;
    r *= sqrt(y);
    x = r*cos(th);
    y = r*sin(th);
}

void CubePosGen::genPos(double* v, double* rnd) const {
    for(AxisDirection d = X_DIRECTION; d <= Z_DIRECTION; ++d)
        v[d] = rnd?rnd[d]:gRandom->Uniform(0,1);
}

void CylPosGen::genPos(double* v, double* rnd) const {
    for(AxisDirection d = X_DIRECTION; d <= Z_DIRECTION; ++d)
        v[d] = rnd?rnd[d]:gRandom->Uniform(0,1);
    square2circle(v[X_DIRECTION],v[Y_DIRECTION],r);
    v[Z_DIRECTION] = (v[Z_DIRECTION]-0.5)*dz;
}
