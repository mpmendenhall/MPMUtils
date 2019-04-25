/// \file testIcosahedral.cc Test of icosahdral point group code
// Michael P. Mendenhall, 2019

#include "Icosahedral.hh"
#include "CodeVersion.hh"
#include <stdlib.h>



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


int main(int, char**) {
    CodeVersion::display_code_version();

    theSieve().display();
    using namespace Icosahedral;
    describe();

    //return EXIT_SUCCESS;

    set<cayley_t::elem_t> S;
    S = {0,1};
    //for(auto& e: dodFaces[0].R) S.insert(e.i);
    quotient_t Q(CT, S);

    std::cout << "Subgroup is " << (isNormal(S,CT)? "" : "*not* ") << "normal in G.\n";

    std::cout << "\n" << Q.order() << " Equivalence classes:\n";
    size_t i = 0;
    for(auto c: Q.EQ) {
        std::cout << "\t" << c.first << "\t" << c.second.size() << "):";
        for(auto x: c.second) std::cout << " " << x;
        std::cout << "\n";
        i += c.second.size();
    }
    std::cout << "(total " << i << " elements).\n";

    for(auto& R: Rs) {
        std::cout << R << "--->\n";
        auto RR = R;
        crude_invert(RR);
        std::cout << RR;
        std::cout << RR*R;
        std::cout << R.inverse() << "\n\n";
    }

    return EXIT_SUCCESS;
}
