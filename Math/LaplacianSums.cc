/// @file LaplacianSums.cc

#include "LaplacianSums.hh"
#include <cmath>
#include <stdio.h>

double sum_laplacian(double c) {
    auto sc = sqrt(fabs(c));
    return c < 0? -M_PI/(tan(M_PI*sc)*sc) : M_PI/(tanh(M_PI*sc) * sc);
}

double sum_factored_quadratic(double u, double d) {
    if(!d) {
        double x = M_PI/sin(M_PI*u);
        return x*x;
    }
    if(!u) return sum_laplacian(-d*d);
    return M_PI*(1/tan(M_PI*(u-d)) - 1/tan(M_PI*(u+d)))/(2*d);
}

double sum_factored_iquadratic(double u, double d) {
    if(!d) {
        double x = M_PI/sin(M_PI*u);
        return x*x;
    }
    if(!u) return sum_laplacian(d*d);
    return M_PI/(d*(1/tanh(2*M_PI*d) - cos(2*M_PI*u)/sinh(2*M_PI*d)));
}

double sum_inverse_quadratic(double a, double b, double c) {
    b/=a;
    c/=a;
    auto u = b*b - 4*c;
    if(u > 0) return sum_factored_quadratic(-0.5*b, 0.5*sqrt(u))/a;
    return sum_factored_iquadratic(-0.5*b, 0.5*sqrt(-u))/a;
}
