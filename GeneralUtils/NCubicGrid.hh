/// \file NCubicGrid.hh Templatized n-dimensional uniform cubic interpolating grid
// This file was produced under the employ of the United States Government,
// and is consequently in the PUBLIC DOMAIN, free from all provisions of
// US Copyright Law (per USC Title 17, Section 105).
// 
// -- Michael P. Mendenhall, 2015

#include <stddef.h>
#include <vector>
using std::vector;
#include <cassert>

// integer power for pre-set array sizes
template<size_t N, size_t M>
constexpr size_t pow_nm() {
    return M? N*pow_nm<N, M? M-1 : M>() : 1;
}


/// Cubic interpolation on N-dimensional uniform grid TODO: boundary conditions unimplemented
template<size_t N, typename T>
class NCubicGrid {
public:
    /// Constructor
    NCubicGrid();
    
    /// boundary conditions for interpolation
    enum IBC {
        IB_CYCLIC,      ///< cyclic edges
        IB_ZERO,        ///< zero-pad edges
        IB_LINEAR,      ///< linear approach to edges
        IB_REPEAT       ///< repeat end value
    } edgeBC[N];        ///< boundary conditions on each axis

    /// set grid dimensions; pre-calculate offsets
    void setDimensions(const size_t d[N]);
    /// set user grid coordinates
    void setUserRange(const T r0[N], const T r1[N], const T e[N]);
    /// set grid point value (adds guard values for boundary conditions as needed)
    void set(const size_t i[N], T v);
    
    /// evaluate at given position
    T operator()(const T x[N]) const;
    /// access (user) grid point value
    T at(const size_t i[N]) { return dat[idx(i) + g_offset]; }
    /// get user coordinate for (user) grid point
    void gridpos(const size_t i[N], T x[N]) const { for(size_t a = 0; a < N; a++) x[a] = (i[a]+2)/sx[a] + ox[a]; }

protected:
    static const size_t N_INTERP_PTS = 4;
    
    vector<T> dat;      ///< data, with guard values
    size_t NX[N];       ///< grid dimensions, not counting guard values
    size_t NStep[N];    ///< step size along each axis
    vector<ptrdiff_t> grid_di;  ///< interpolation grid points precalculated offsets
    size_t g_offset;    ///< offset to skip guard points
    T sx[N];            ///< user coordinates scale for each dimension
    T ox[N];            ///< user coordinates offset for each dimension
    
    /// index for coordinate
    size_t idx(const size_t i[N]) const;
    /// interpolate point in grid coordinates
    inline T eval_interpolated(const T x[N]) const;
};

/*
class BicubicGrid: public NCubicGrid<2,double> {
public:
    BicubicGrid();
};
*/

////////////////////////////////////////////
////////////////////////////////////////////
////////////////////////////////////////////

/// increment N-dimensional counter with max M on each digit
template<size_t N, size_t M>
bool increment_counter(size_t c[N]) {
    size_t i = 0;
    for(; i<N; i++) {
        c[i]++;
        if(c[i] >= M) c[i] = 0;
        else break;
    }
    return i < N;
}

/// increment N-dimensional counter with variable limit 
template<size_t N>
bool increment_counter(size_t c[N], const size_t m[N]) {
    size_t i = 0;
    for(; i<N; i++) {
        c[i]++;
        if(c[i] >= m[i]) c[i] = 0;
        else break;
    }
    return i < N;
}

template<size_t N, typename T>
NCubicGrid<N,T>::NCubicGrid() {
    size_t d[N];
    for(size_t a = 0; a < N; a++) {
        edgeBC[a] = IB_CYCLIC;
        ox[a] = -2;
        sx[a] = 1;
        d[a] = 0;
    }
    setDimensions(d);
}

template<size_t N, typename T>
void NCubicGrid<N,T>::setDimensions(const size_t d[N]) {
    // calculate dimension offsets
    size_t ns = 1;
    g_offset = 0;
    for(size_t a = 0; a < N; a++) {
        NX[a] = d[a];
        NStep[a] = ns;
        g_offset += 2*ns;
        ns *= d[a]+4;
    }
    dat.resize(ns);
    
    // data offsets for sample points
    grid_di.clear();
    size_t c[N];
    ptrdiff_t i0 = 0;
    for(size_t a = 0; a < N; a++) {
        c[a] = 0;
        i0 -= NStep[a];
    }
    do {
        ptrdiff_t i = i0;
        for(size_t a = 0; a < N; a++) i += NStep[a]*c[a];
        grid_di.push_back(i);
    } while (increment_counter<N,N_INTERP_PTS>(c));
}

template<size_t N, typename T>
void NCubicGrid<N,T>::setUserRange(const T r0[N], const T r1[N], const T e[N]) {
    // solution to sx*(r0-ox) = 2-e; sx*(r1-ox) = NX+1+e
    for(size_t a = 0; a < N; a++) {
        sx[a] = (NX[a]-1+2*e[a])/(r1[a]-r0[a]);
        ox[a] = ((NX[a]+1+e[a])*r0[a]-(2-e[a])*r1[a])/(NX[a]-1+2*e[a]);
    }
}

template<size_t N, typename T>
void NCubicGrid<N,T>::set(const size_t i[N], T v) {
    size_t ii = idx(i);
    dat[ii + g_offset] = v;
    // TODO set guard points for boundary conditions        
}

template<size_t N, typename T>
size_t NCubicGrid<N,T>::idx(const size_t i[N]) const {
    size_t c = 0;
    for(size_t a=0; a<N; a++)
        c += i[a]*NStep[a];
    return c;
}

template<size_t N, typename T>
T NCubicGrid<N,T>::operator()(const T x[N]) const {
    // convert user coordinates to gaurded grid coords
    T xx[N];
    for(size_t a = 0; a < N; a++) xx[a] = sx[a]*(x[a]-ox[a]);
    return eval_interpolated(xx);
}

template<typename T>
inline T sum4coeffs(const T* dat, const T* coeffs) {
    return dat[0]*coeffs[0] + dat[1]*coeffs[1] + dat[2]*coeffs[2] + dat[3]*coeffs[3];
}

template<typename T, size_t N>
inline T sumreduce_block(const T* dat, const T coeffs[N*4]) {
    if(N==1) return sum4coeffs<T>(dat, coeffs);
    
    T datreduced[pow_nm<4,N? N-1 : 0>()];
    for(size_t i=0; i<pow_nm<4,N? N-1 : 0>(); i++)
        datreduced[i] = sum4coeffs<T>(dat+4*i, coeffs);
    return sumreduce_block<T,N? N-1 : 0>(datreduced, coeffs+4);
}

template<typename T, size_t N>
inline void transfer4N(const T* dIn, T* dOut, const size_t stride[N]) {
    if(N==1) {
        for(int i=0; i<4; i++) dOut[i] = dIn[i];
        return;
    }
    for(int i=0; i<4; i++) transfer4N<T,N? N-1 : 0>(dIn + i*stride[N-1], dOut+i*pow_nm<4,N? N-1 : 0>(), stride);
}

template<size_t N, typename T>
inline T NCubicGrid<N,T>::eval_interpolated(const T x[N]) const {
    // integral and fractional coordinates; data start index
    int ix[N];              // integer coordinates, including guard offset
    T fx[N];                // fractional coordinates
    const T* i0 = dat.data();     // data[-1,-1,-1...] pointer
    for(size_t a = 0; a < N; a++) {
        ix[a] = int(x[a]);
        fx[a] = x[a]-ix[a];
        if(fx[a]<0) { fx[a] += 1; ix[a] -= 1; }
        
        // range check
        if(ix[a] < 1 || ix[a] > int(NX[a]+1)) {
            //if(edgeBC[a]==IB_CYCLIC) ix[a] = ((ix[a]-2)+100*NX[a])%NX[a] + 2;
            return 0;
        }
        assert(1<=ix[a] && ix[a]<=int(NX[a]+1));
        
        i0 += (ix[a]-1)*NStep[a];
    }
    
    // cubic interpolating polynomials at each axis' fractional component
    T px[N*N_INTERP_PTS];
    for(size_t a = 0; a < N; a++) {
        const T& x = fx[a];
        const T xx = x*x;
        const T xxx = x*xx;
        px[a*4+0] = -0.5*(x - 2*xx + xxx);
        px[a*4+1] = 1 - 2.5*xx + 1.5*xxx;
        px[a*4+2] = 0.5*x + 2*xx - 1.5*xxx;
        px[a*4+3] = 0.5*(xxx-xx);
    }
    
    T interpdat[pow_nm<4,N>()];
    transfer4N<T,N>(i0, interpdat, NStep);
    return sumreduce_block<T,N>(interpdat, px);
}
