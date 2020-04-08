/// \file testRelKin.cc Test of relativistic kinematics calcs

#include "CodeVersion.hh"
#include "RelKin.hh"
#include <stdio.h>

int main(int, char**) {
    CodeVersion::display_code_version();

    printf("\n");
    testRelKin();

    printf("\n\nEnergy/momentum conversions, numerically stable in nonrel. limit:\n");
    for(double m: {0., 1., 10., 100., 1e4, 1e6, 1e7, 1e8, 1e9, 1e99}) {
        double x = sqrt(1. + m*m) - m;
        printf("p = 1, m = %g\tKE = %g\t(naive: %g)\n", m, p_to_KE(1., m), x);
    }

    printf("\n\nbeta/gamma conversions, numerically stable in nonrel. limit:\n");
    for(double b: {0., 1e-2, 1e-5, 1e-6, 1e-7, 1e-8, 1e-9, 1e-99}) {
        auto gm1 = beta_to_gammaM1(b);
        printf("beta = %g\tgamma = 1 + %g\t(naive: 1 + %g)\troundtrip error %g\n",
               b, gm1, beta_to_gamma(b)-1, (gammaM1_to_beta(gm1)-b)/(b? b : 1));
    }

    printf("\n\nBoost composition round-trips:\n");
    auto L0 = Lorentz_boost::from_beta(0.8);
    L0.display();
    (L0 * L0.inverse()).display();
    (L0.inverse() * L0).display();
    (L0 / L0).display();

    return EXIT_SUCCESS;
}
