/// @file LaplacianSums.hh Infinite sums of 1/quadratic form
// Michael P. Mendenhall, LLNL 2021

/// \f$\sum_{k=-\infty}^{\infty} 1/(k^2 + c)\f$
double sum_laplacian(double c);

/// \f$\sum_{k=-\infty}^{\infty} 1/(a k^2 + c)\f$
inline double sum_laplacian(double a, double c) { return sum_laplacian(c/a)/a; }

/// \f$\sum_{k=-\infty}^{\infty} 1/(k + u + d)(k + u - d)\f$
double sum_factored_quadratic(double u, double d);

/// \f$\sum_{k=-\infty}^{\infty} 1/(k + u + di)(k + u - di)\f$
double sum_factored_iquadratic(double u, double d);

/// \f$\sum_{k=-\infty}^{\infty} 1/(a k^2 + b k + c)\f$
double sum_inverse_quadratic(double a, double b, double c);

