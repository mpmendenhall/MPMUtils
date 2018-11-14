/// \file testQuadratic.cc Test quadratic manipulations

#include "CodeVersion.hh"
#include "Quadratic.hh"
#include <vector>
using std::vector;

int main(int, char**) {
    CodeVersion::display_code_version();

    Quadratic<3> Q;
    Q.display();
    Quadratic<3> R(vector<double>({1.,2.,3.,4.,5.,6.,7.,8.,9.,10.}));
    R += R;
    R.factor();
    R.display();

    vector<double> x0 = {-5.7, -0.6, 1.4};
    printf("%g\n", R(x0));

    array<double,10> c;
    Quadratic<3>::evalTerms(x0, c);
    for(auto x: c) printf("\t%g",x);
    printf("\n");

    return EXIT_SUCCESS;
}
