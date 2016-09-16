/// \file SatoNiitaNeutrons.cc
// This file was produced under the employ of the United States Government,
// and is consequently in the PUBLIC DOMAIN, free from all provisions of
// US Copyright Law (per USC Title 17, Section 105).
//
// -- Michael P. Mendenhall, 2015

#include "SatoNiitaNeutrons.hh"
#include <cmath>
#include <cassert>
#include <iostream>

void SatoNiitaNeutrons::setParameters(double ss, double rc, double d, double w) {
    s_mod = ss;
    r_c = rc;
    depth = d;
    calcFluxNorm();

    waterFrac = w;
}

void SatoNiitaNeutrons::calcFluxNorm() {
    // Eq. (6): b_i1, b_i2 interpolated values between solar min and max
    b_i1[1] = (b_11mn*(s_max - s_mod) + b_11mx*(s_mod - s_min))/(s_max - s_min);
    for(int i=1; i<=4; i++)
        b_i2[i] = (b_i2mn[i]*(s_max - s_mod) + b_i2mx[i]*(s_mod - s_min))/(s_max - s_min);

    // Eq. (5)
    for(int i=1; i<=5; i++)  a[i] = b_i1[i] + b_i2[i]/(1. + exp((r_c - b_i3[i])/b_i4[i]));
    for(int i=9; i<=11; i++) a[i] = b_i1[i] + b_i2[i]/(1. + exp((r_c - b_i3[i])/b_i4[i]));

    // Eq. (8)
    c_4 = a[5] + (a[6]*depth / (1 + a[7]*exp(a[8]*depth)));
    // Eq. (9)
    c_12 = a[9]*(exp(-a[10]*depth) + a[11]*exp(-a[12]*depth));

    // Eq. (4)
    phi_L = a[1]*(exp(-a[2]*depth) - a[3]*exp(-a[4]*depth));
}

double SatoNiitaNeutrons::calcAirSpectrum(double E) {
    // Eq. (7) for phi_B-bar, with variable c_4 and c_12
    phi_B = ( c_1*pow(E/c_2,c_3)*exp(-E/c_2)
              + c_4*exp(-pow(log10(E/c_5),2) / (2*pow(log10(c_6),2)))
              + c_7*log10(E/c_8) * (1 + tanh(c_9*log10(E/c_10))) * (1 - tanh(c_11*log10(E/c_12))) )*scale_S;

    // Eq. (2)
    return phi_inf = phi_B*phi_L;
}

double SatoNiitaNeutrons::calcGroundSpectrum(double E) {
    calcAirSpectrum(E);

    double w = waterFrac;

    // Eq. (12), corrected with missing "+"
    g_3 = pow(10., h_31 + h_32/(w+h_33))*MeV;
    // Eq. (13)
    g_5 = (h_51 + h_52*w + h_53*w*w)*MeV;
    // Eq. (16)
    g_6 = (h_61 + h_62*exp(-h_63*w)) / (1 + h_64*exp(-h_65*w));

    // Eq. (11)
    f_G = pow(10., g_1 + g_2*log10(E/g_3)*(1-tanh(g_4*log10(E/g_5))));

    // Eq. (14)
    phi_T = g_6*E*E/(E_T*E_T)*exp(-E/E_T);
    phi_T_scaled = phi_L*phi_T*scale_T;

    // Eq. (10)
    return phi_G = phi_L*phi_B*f_G + phi_T_scaled;
}
