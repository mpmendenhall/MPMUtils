#include "ConvolverFactory.hh"
#include <cmath>
#include <cstdio>
#include <cassert>

using std::pair;

bool better(const vector<int>& a, const vector<int>& b) {
    assert(a.size()==b.size());
    for(int i = a.size(); i>0; i--)
        if(a[i-1] < b[i-1]) return true;
    return false;
}
    
int fftw_best_nearest_size(int i, int dmax) {
    static map< pair<int,int>, int > m;
    
    auto it = m.find(pair<int,int>(i,dmax));
    if(it != m.end()) return it->second;
    
    int jbest = i;
    const int factors[6] = {2,3,5,7,11,13};
    vector<int> powcount(7);
    vector<int> bestcount(7);
    bestcount.back() = i;
    for(int j = i; j > i-dmax; j--) {
        int resid = j;
        for(int n=0; n<6; n++) while(!(resid%factors[n])) { powcount[n]++; resid /= factors[n]; }
        powcount.back() = resid;
        if(better(powcount,bestcount)) { bestcount = powcount; jbest = j; } 
    }
    m.insert(pair< pair<int,int>, int >(pair<int,int>(i,dmax),jbest));
    return jbest;
}

map<unsigned int,Convolver_FFT*> Convolver_FFT::ffters;

Convolver_FFT::Convolver_FFT(unsigned int m): M(m), realspace(new double[M]), kspace(new complex<double>[M/2+1]) {
    FILE* fin = fopen("fftw_wisdom","r");
    if(fin) {
        fftw_import_wisdom_from_file(fin);
        fclose(fin);
    }
    
    forwardplan = fftw_plan_dft_r2c_1d(M,
                                       realspace,
                                       (fftw_complex*)kspace,
                                       FFTW_MEASURE);
    reverseplan = fftw_plan_dft_c2r_1d(M,
                                       (fftw_complex*)kspace,
                                       realspace,
                                       FFTW_MEASURE);
    
    printf("Calculating convolver plan for N=%u...",m);
    fflush(stdout);
    
    FILE* fout = fopen("fftw_wisdom","w");
    if(fout) {
        fftw_export_wisdom_to_file(fout);
        fclose(fout);
    }
    printf("Done.\n");
}

//////////////////////
//////////////////////
//////////////////////

Convolver_FFT& Convolver_FFT::get_ffter(unsigned int m) {
    auto it = ffters.find(m);
    if(it != ffters.end()) return *(it->second);
    Convolver_FFT* f = new Convolver_FFT(m);
    ffters.insert(pair<unsigned int, Convolver_FFT*>(m,f));
    return *f;
}

void ConvolverFactory::convolve(vector<double>& v) {
    auto kern = getKernel(v.size());
    Convolver_FFT& ffter = Convolver_FFT::get_ffter(v.size());
    std::copy(v.begin(), v.end(), ffter.realspace);
    fftw_execute(ffter.forwardplan);
    for(size_t i=0; i<kern.size(); i++) ffter.kspace[i] *= kern[i];
    fftw_execute(ffter.reverseplan);
    for(size_t i=0; i<v.size(); i++) v[i] = ffter.realspace[i]/double(v.size());
}

const vector< complex<double> >& ConvolverFactory::getKernel(unsigned int i) {
    auto it = kdata.find(i);
    if(it != kdata.end()) return it->second;
    
    it = kdata.insert(pair<unsigned int, vector< complex<double> > >(i, vector< complex<double> >(i/2+1))).first;
    vector<double> kern = calcKernel(i);
    
    Convolver_FFT& ffter = Convolver_FFT::get_ffter(i);
    std::copy(kern.begin(), kern.end(), ffter.realspace);
    fftw_execute(ffter.forwardplan);
    std::copy(ffter.kspace, ffter.kspace+i/2+1, it->second.begin());
    return it->second;
}

//////////////////////
//////////////////////
//////////////////////

vector<double> GaussConvolverFactory::calcKernel(unsigned int i) const {
    vector<double> v(i);
    double nrm = 0;
    for(unsigned int n=0; n<i; n++) {
        v[n] = exp(-pow((n<i/2? n : i-n)/r,2)/2);
        nrm += v[n];
    }
    for(unsigned int n=0; n<i; n++) v[n] /= nrm;
    return v;
}

vector<double> TemplateConvolverFactory::calcKernel(unsigned int i) const {
    vector<double> vs(i);
    assert(s0 >= 0 && s0 < (int)v.size());
    for(int s=s0; s>=std::max(0,int(s0-i+1)); s--) vs[s0-s] = v[s];
    for(int s=s0+1; s<(int)std::min(v.size(),i); s++) vs[i-(s-s0)] = v[s];
    return vs;
}

void TemplateConvolverFactory::flip() {
    vector<double> vflip(v.size());
    vflip[0] = v[0];
    for(size_t i=1; i<v.size(); i++) vflip[i] = v[v.size()-i];
    v = vflip;
}

