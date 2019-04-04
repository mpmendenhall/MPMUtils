/// \file testMatrix.cc Test matrix operations

#include "CodeVersion.hh"
#include "Rational.hh"
#include "Matrix.hh"
#include "Stopwatch.hh"

template<typename T>
void mtest() {
    typedef Matrix<7,7,T> Mat4;
    Mat4 M = Mat4::identity();

    for(auto& c: M) c += rand()%10;

    LUPDecomp<7,T> L;
    {
        Stopwatch w;
        for(int i=0; i<10000; i++) L = LUPDecomp(M);
    }

    std::cout << M << "\n" << L.L() << "\n" << L.U() << "\n";

    Mat4 Mi;
    {
        Stopwatch w;
        for(int i=0; i<5000; i++) L.inverse(Mi);
    }
    std::cout << M*Mi << "\n";

    Mat4 Mi2;
    {
        Stopwatch w;
        for(int i=0; i<5000; i++) { Mi2 = M; Mi2.invert(); }
    }
    std::cout << Mi2*M << "\n";

    std::cout << det(M) << "\t" << L.det() << "\n";
}

int main(int, char**) {
    CodeVersion::display_code_version();

    mtest<float>();
    mtest<double>();
    mtest<Rational>();

    return EXIT_SUCCESS;
}
