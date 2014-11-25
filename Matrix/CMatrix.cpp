/* 
 * CMatrix.cpp, part of the RotationShield program
 * Copyright (c) 2007-2014 Michael P. Mendenhall
 *
 * This code uses the FFTW3 library for Fourier transforms, http://www.fftw.org/
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

#include "CMatrix.hh"


#include <stdlib.h>
#include <iomanip>
#include <algorithm>

using std::cout;

// FFT handler ------______------______-------_______------______------

map<unsigned int,cmatrix_fft*> cmatrix_fft::ffters;

cmatrix_fft::cmatrix_fft(unsigned int m): M(m), realspace(new double[M]), kspace(new complex<double>[M/2+1]) {
    FILE* fin = fopen("fftw_wisdom","r");
    if(fin) {
        fftw_import_wisdom_from_file(fin);
        fclose(fin);
    }
    
    forwardplan = fftw_plan_dft_r2c_1d(M,
        realspace,
        (fftw_complex*)kspace,
                                       FFTW_EXHAUSTIVE);
    reverseplan = fftw_plan_dft_c2r_1d(M,
                                       (fftw_complex*)kspace,
                                       realspace,
                                       FFTW_EXHAUSTIVE);
    
    FILE* fout = fopen("fftw_wisdom","w");
    if(fout) {
        fftw_export_wisdom_to_file(fout);
        fclose(fout);
    }
}

cmatrix_fft& cmatrix_fft::get_ffter(unsigned int m) {
    auto it = ffters.find(m);
    if(it != ffters.end()) return *(it->second);
    cmatrix_fft* f = new cmatrix_fft(m);
    ffters.insert(std::pair<unsigned int, cmatrix_fft*>(m,f));
    return *f;
}

// File IO ------______------______-------_______------______------

void CMatrix::writeToFile(ostream& o) const {
    writeString("(CMatrix)",o);
    o.write((char*)&M,                  sizeof(M));
    o.write((char*)&has_realspace,      sizeof(has_realspace));
    o.write((char*)&has_kspace,         sizeof(has_kspace));
    
    if(has_realspace)
        o.write((char*)&data[0],        sizeof(data[0])*M);
    if(has_kspace)
        o.write((char*)&kdata[0],       sizeof(kdata[0])*(M/2+1));
    writeString("(/CMatrix)",o);
}


CMatrix CMatrix::readFromFile(std::istream& s) {
    checkString("(CMatrix)",s);
    CMatrix foo;
    s.read((char*)&foo.M,               sizeof(foo.M));
    s.read((char*)&foo.has_realspace,   sizeof(foo.has_realspace));
    s.read((char*)&foo.has_kspace,      sizeof(foo.has_realspace));
    
    if(foo.has_realspace) {
        foo.data.resize(foo.M);
        s.read((char*)&foo.data[0],     sizeof(foo.data[0])*foo.M);
    }
    if(foo.has_kspace) {
        foo.kdata.resize(foo.M/2+1);
        s.read((char*)&foo.kdata[0],    sizeof(foo.kdata[0])*(foo.M/2+1));
    }
    checkString("(/CMatrix)",s);
    return foo;
}

// Special Matrices ------______------______-------_______------______------

CMatrix CMatrix::identity(unsigned int M) {
    CMatrix m(M);
    if(!M) return m;
    m[0] = 1.0;
    return m;
}

CMatrix CMatrix::random(unsigned int M) {
    CMatrix m(M);
    for(unsigned int i=0; i<M; i++)
        m[i] = double(rand())/double(RAND_MAX);
    return m;
}

// Data Providers ------______------______-------_______------______------

void CMatrix::zero() const {
    has_kspace = true;
    has_realspace = true;
    std::fill(data.begin(), data.end(), 0);
    std::fill(kdata.begin(), kdata.end(), 0);
}

void CMatrix::calculateKData() const {
    assert(has_realspace);
    cmatrix_fft& ffter = cmatrix_fft::get_ffter(M);
    std::copy(data.begin(), data.end(), ffter.realspace);
    fftw_execute(ffter.forwardplan);
    std::copy(ffter.kspace, ffter.kspace+M/2+1, kdata.begin());
    has_kspace = true;
}

void CMatrix::calculateRealData() const {
    assert(has_kspace);
    cmatrix_fft& ffter = cmatrix_fft::get_ffter(M);
    std::copy(kdata.begin(), kdata.end(), ffter.kspace);
    fftw_execute(ffter.reverseplan);
    for(unsigned int n=0; n<M; n++) data[n] = ffter.realspace[n]/double(M);
    has_realspace = true;
}

double CMatrix::operator[](unsigned int i) const {
    assert(i<M);
    if(!has_realspace)
        calculateRealData();
    return data[i];
}

double& CMatrix::operator[](unsigned int i) {
    assert(i<M);
    if(!has_realspace)
        calculateRealData();
    has_kspace = false; // messing with real space data invalidates kspace data
    return data[i];
}

vector< complex<double> >& CMatrix::getKData() {
    if(!has_kspace)
        calculateKData();
    has_realspace = false; // messing with kspace data invalidates real data
    return kdata;
}

const vector< complex<double> >& CMatrix::getKData() const {
    if(!has_kspace)
        calculateKData();
    return kdata;
}

vector<double>& CMatrix::getRealData() {
    if(!has_realspace)
        calculateRealData();
    has_kspace = false; // messing with real space data invalidates kspace data
    return data;
}

const vector<double>& CMatrix::getRealData() const {
    if(!has_realspace)
        calculateRealData();
    return data;
}

// Matrix properties ------______------______-------_______------______------


double CMatrix::norm_L2() const {
    const vector< complex<double> >& v = getKData();
    vector<double> vn;
    for(unsigned int i=0; i<v.size(); i++)
        vn.push_back(abs(v[i]));
    return *std::max_element(vn.begin(),vn.end());
}


double CMatrix::det() const {
    if(!M) return 0;
    const vector< complex<double> >& v = getKData();
    double d = v[0].real();
    for(unsigned int i=1; i<v.size(); i++)
        d *= norm(v[i]);
    if(!(M%2)) d /= v[M/2].real();
    return d;
}


double CMatrix::trace() const {
    if(!M) return 0;
    if(has_realspace) return M*data[0];
    else if(has_kspace) {
        double s = kdata[0].real();
        for(unsigned int i=1; i < kdata.size(); i++)
            s += kdata[i].real()*2;
        if(!(M%2)) s -= kdata[M/2].real();
        return s;
    } else return 0;
}

// "GUI" ------______------______-------_______------______------


void CMatrix::printRow(int r) const {
    printf("| ");
    for(unsigned int i=0; i<M; i++) printf("%.3g ",(*this)[(i+(M-r))%M]);
    printf("|");
}


void CMatrix::display() const {
    cout << "CMatrix " << M << " " << has_realspace << " " << has_kspace << std::endl;
    for(unsigned int r=0; r<M; r++) {
        printRow(r);
        printf("\n");
    }
    cout << "CMatrix " << M << " " << has_realspace << " " << has_kspace << std::endl;
}


void CMatrix::displayK() const {
    cout << "{ ";
    for(unsigned int i=0; i<M/2+1; i++) cout << getKData()[i] << " ";
    cout << "}" << std::endl;
}


ostream& operator<<(ostream& o, const CMatrix& m) {
    for(unsigned int r=0; r<m.nRows(); r++) {
        o << "| ";
        for(unsigned int c=0; c<m.nCols(); c++)
            o << m[(c+(m.nRows()-r))%m.nCols()] << " ";
        o << "|\n";
    }
    return o;
}


// Matrix Ops ------______------______-------_______------______------


CMatrix& CMatrix::operator+=(const CMatrix& m) {
    
    assert(m.nRows()==nRows());
    assert(has_realspace || has_kspace);
    assert(m.has_realspace || m.has_kspace);
    
    if(has_kspace && (m.has_kspace || !has_realspace)) {
        const vector< complex<double> >& kd = m.getKData();
        for(unsigned int i=0; i<kdata.size(); i++)
            kdata[i] += kd[i];
    } else has_kspace = false;
    
    if(has_realspace && (m.has_realspace || !has_kspace)) {
        const vector<double>& d = m.getRealData();
        for(unsigned int i=0; i<data.size(); i++)
            data[i] += d[i];
    } else has_realspace = false;
    
    return *this;
}


const CMatrix CMatrix::operator+(const CMatrix& m) const {
    CMatrix r = *this;
    r += m;
    return r;
}


CMatrix& CMatrix::operator-=(const CMatrix& m) {
    
    assert(m.nRows()==nRows());
    assert(has_realspace || has_kspace);
    assert(m.has_realspace || m.has_kspace);
    
    if(has_kspace && (m.has_kspace || !has_realspace)) {
        const vector< complex<double> >& kd = m.getKData();
        for(unsigned int i=0; i<kdata.size(); i++)
            kdata[i] -= kd[i];
    } else has_kspace = false;
    
    if(has_realspace && (m.has_realspace || !has_kspace)) {
        const vector<double>& d = m.getRealData();
        for(unsigned int i=0; i<data.size(); i++)
            data[i] -= d[i];
    } else has_realspace = false;
    
    return *this;
}


const CMatrix CMatrix::operator-(const CMatrix& m) const {
    CMatrix r = *this;
    r -= m;
    return r;
}


CMatrix& CMatrix::operator*=(double c) {
    
    if(has_realspace)
        for(unsigned int i=0; i<data.size(); i++) data[i] *= c;
        
        if(has_kspace)
            for(unsigned int i=0; i<kdata.size(); i++) kdata[i] *= c;
            
            return *this;
}


const CMatrix CMatrix::operator*(double c) const {
    CMatrix r = *this;
    r *= c;
    return r;
}


const CMatrix CMatrix::operator-() const {
    CMatrix r = *this;
    r *= -1.0;
    return r;
}


CMatrix& CMatrix::operator*=(const CMatrix& m) {
    
    assert(m.nRows()==nRows());
    
    vector< complex<double> >& kd = getKData();
    const vector< complex<double> >& mkd = m.getKData();
    for(unsigned int i=0; i<kd.size(); i++) kd[i] *= mkd[i];
    
    return *this;
}

const CMatrix CMatrix::operator*(const CMatrix& m) const {
    CMatrix r = *this;
    r *= m;
    return r;
}


const VarVec<double> CMatrix::operator*(const VarVec<double>& v) const {
    
    assert(M>0 && v.size()==M);
    const vector< complex<double> >& kd = getKData(); // make sure to do this first, since we need ffter's storage space after
    
    cmatrix_fft& ffter = cmatrix_fft::get_ffter(M);
    ffter.realspace[0] = v[0];
    for(unsigned int i=1; i<M; i++) ffter.realspace[i] = v[M - i];
    fftw_execute(ffter.forwardplan);
    
    for(unsigned int i=0; i<kd.size(); i++) ffter.kspace[i] *= kd[i];
    fftw_execute(ffter.reverseplan);
    
    VarVec<double> out = VarVec<double>(M);
    out[0] = ffter.realspace[0];
    for(unsigned int i=1; i<M; i++) out[i] = ffter.realspace[M-i];
    out /= double(M);
    return out;
}


CMatrix& CMatrix::invert() {
    vector< complex<double> >& kd = getKData();
    for(unsigned int i=0; i<kdata.size(); i++)
        kd[i] = 1./kd[i];
    has_realspace = false;
    return *this;
}


const CMatrix CMatrix::inverse() const {
    CMatrix M = *this;
    M.invert();
    return M;
}

const CMatrix CMatrix::transpose() const {
    CMatrix m(M);
    if(!M) return m;
    m[0] = (*this)[0];
    for(unsigned int n=1; n<M; n++) m[n] = (*this)[M-n];
    return m;
}
