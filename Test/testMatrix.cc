/// \file testMatrix.cc Test matrix operations

#include "CodeVersion.hh"
#include "Rational.hh"
#include "Matrix.hh"
#include "Stopwatch.hh"


namespace matrix_element_inversion {

    template<typename T>
    inline void invert_element(T& t) { t.invert(); }
    template<>
    inline void invert_element(float& t) { t = 1.0/t; }
    template<>
    inline void invert_element(double& t) { t = 1.0/t; }

}

template<size_t M, typename T>
void crude_invert(Matrix<M,M,T>& X, size_t n = 0) {

    // invert the first cell
    T& firstcell = X(n,n);
    matrix_element_inversion::invert_element(firstcell);
    for(size_t i=n+1; i<M; i++) X(n,i) *= firstcell;

    // use to clear first column
    for(size_t r=n+1; r<M; r++) {
        T& m0 = X(r,n);
        for(size_t c=n+1; c<M; c++)
            X(r,c) -= X(n,c)*m0;
        m0 *= -firstcell;
    }

    if(n == M-1) return;

    //invert the submatrix
    crude_invert(X,n+1);

    // temporary space allocation
    vector<T> subvec = vector<T>(M-n-1);

    // first column gets multiplied by inverting submatrix
    for(size_t r=n+1; r<M; r++)
        subvec[r-n-1] = X(r,n);
    for(size_t r=n+1; r<M; r++) {
        X(r,n) = X(r,n+1)*subvec[0];
        for(size_t c=n+2; c<M; c++)
            X(r,n) += X(r,c)*subvec[c-n-1];
    }

    //finish off by cleaning first row
    for(size_t c=n+1; c<M; c++)
        subvec[c-n-1] = X(n,c);
    for(size_t c=n; c<M; c++) {
        if(c>n)
            X(n,c) = -X(n+1,c) * subvec[0];
        else
            X(n,c) -= X(n+1,c) * subvec[0];
        for(size_t r=n+2; r<M; r++)
            X(n,c) -= X(r,c) * subvec[r-n-1];
    }
}


template<typename T, size_t N>
void mtest(bool do_crude = false) {
    typedef Matrix<N,N,T> Mat_t;
    typedef LUPDecomp<N,T> LU_t;

    std::cout << "--------------------------------------------\n";

    Mat_t I = Mat_t::identity();
    Mat_t M = I;
    for(auto& c: M) c += (rand()%11)-5;

    LU_t L;
    {
        Stopwatch w;
        for(int i=0; i<5000; i++) {
            Mat_t MM = I;
            for(auto& c: MM) c += (rand()%15)-7;
            L = LU_t(MM);
        }
    }
    L = LU_t(M);
    Mat_t Mi;
    L.inverse(Mi);
    std::cout << M << "\n" << L.L() << "\n" << L.U() << "\n";

    {

        Stopwatch w;
        for(int i=0; i<5000; i++) {
            Mat_t MM = I;
            for(auto& c: MM) c += (rand()%15)-7;
            LU_t Lx(MM);
            if(!Lx.isSingular()) {
                auto MM0 = MM;
                Lx.inverse(MM);
                assert(MM0*MM == I);
            }
        }
    }

    std::cout << M*Mi << "\n";

    {
        Stopwatch w;
        for(int i=0; i<5000; i++) {
            Mat_t MM = I;
            for(auto& c: MM) c += (rand()%15)-7;
            LU_t Lx(MM);
            Lx.det();
        }
    }

    auto d = det(M);
    std::cout << "Det = " << d << "\n\n";

    if(!do_crude) return;

    {
        Stopwatch w;
        for(int i=0; i<5000; i++) {
            Mat_t MM = I;
            for(auto& c: MM) c += (rand()%15)-7;
            crude_invert(MM);
        }
    }
    Mat_t Mi2 = M;
    crude_invert(Mi2);
    std::cout << Mi2*M << "\n";

}

int main(int, char**) {
    CodeVersion::display_code_version();

    //mtest<float,  11>(true);
    //mtest<double, 11>(true);
    mtest<Rational, 6>();

    return EXIT_SUCCESS;
}
