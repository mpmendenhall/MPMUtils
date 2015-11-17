/// \file UnpolarizedNeutronDecay.cc
// This file was produced under the employ of the United States Government,
// and is consequently in the PUBLIC DOMAIN, free from all provisions of
// US Copyright Law (per USC Title 17, Section 105).
//
// -- Michael P. Mendenhall, 2015

#include "UnpolarizedNeutronDecay.hh"
#include <stdio.h>

void NeutronDecayKinematics::calc_proton() {
    p_1 = E_1; // massless neutrino approximation
    mag_p_f = 0;
    for(int i=0; i<3; i++) {
        p_f[i] = -n_1[i]*p_1 - n_2[i]*p_2 - n_gamma[i]*K;
        mag_p_f += p_f[i]*p_f[i];
    }
    mag_p_f = sqrt(mag_p_f);
}

void NeutronDecayKinematics::n_from_angles(double c, double phi, double n[3]) {
    const double s = sqrt(1-c*c);
    n[0] = s*cos(phi);
    n[1] = s*sin(phi);
    n[2] = c;
}

double NeutronDecayKinematics::proton_ctheta() const {
    double pep[3];
    for(int i=0; i<3; i++) pep[i] = n_2[i]*p_2 + p_f[i];
    return -dot3(pep, n_2)/sqrt(dot3(pep,pep));
}

double NeutronDecayKinematics::Gluck93_radcxn_wt() const {
    double c = proton_ctheta();
    double x = (E_2-m_2)/(Delta-m_2);
    return 1 + Wilkinson_g_a2pi(E_2/m_2) + 0.01*Gluck93_r_enu(x,c);
}

double NeutronDecayKinematics::B59_rwm_cxn_wt() const {
    return B59_rwm_cxn(E_2, cos_theta_e_nu());
}

////////////////////////////////////
////////////////////////////////////

N3BodyUncorrelated::N3BodyUncorrelated(NKine_Rndm_Src* R): NeutronDecayKinematics(R) {
    // build cumulative density table
    double cdf[NPTS];
    double nrm = plainPhaseSpaceCDF(beta_W0);
    for(size_t i = 0; i < NPTS; i++) {
        double W = 1 + double(i)/double(NPTS-1)*(beta_W0-1);
        cdf[i] = plainPhaseSpaceCDF(W)/nrm;
    }
    cdf[0] = -1e-6;
    cdf[NPTS-1] = 1.000001;
    
    // linearly interpolate inverse table, plus guard values
    size_t j = 1;
    invcdf[0] = 0;
    for(size_t i = 1; i < NPTS; i++) {
        double x = double(i)/double(NPTS-1);
        while(x > cdf[j]) j++; // make cdf[j] >= x
        double f = (cdf[j]-x)/(cdf[j]-cdf[j-1]);
        invcdf[i+2] = neutronBetaEp*(j-f)/double(NPTS-1);
    }
    invcdf[1] = -2*invcdf[3];
    invcdf[NPTS+1] = neutronBetaEp;
    invcdf[NPTS+2] = 6*neutronBetaEp - 5*invcdf[NPTS];
    invcdf[NPTS+3] = 1e9;
}

double eval_cubic_interpl(double y, const double* d) {
    return ( -0.5*d[0]*(1-y)*(1-y)*y
    +d[1]*(1-y)*(1-y*(1.5*y-1))
    -d[2]*y*(-0.5*(1-y)*(1-y)+y*(2*y-3))
    -0.5*d[3]*(1-y)*y*y );
}

void N3BodyUncorrelated::gen_evt_weighted() {
    assert(myR);
    myR->next(); // random seed
    evt_w = 1;
    
    // electron energy cubic interpolated from inverse CDF lookup table
    double jf = myR->u[0]*(NPTS-1);
    size_t j = size_t(jf);
    jf -= j;
    E_2 = m_2 + eval_cubic_interpl(jf, invcdf + j + 1);
    
    // electron momentum magnitude, velocity
    p_2 = sqrt(E_2*E_2 - m_2*m_2);
    beta = sqrt(1-m_2*m_2/E_2/E_2);
    
    // electron direction, including transverse momentum limiting
    c_2_min = -1;
    if(pt2_max && p_2 > pt2_max)
        c_2_min = sqrt(1.-pt2_max*pt2_max/(p_2*p_2));
    c_2_wt = (1-c_2_min)/2;
    c_2 = c_2_min + (1-c_2_min)*myR->u[1];
    phi_2 = 2*M_PI*myR->u[2];
    n_from_angles(c_2, phi_2, n_2);
    
    // neutrino energy, direction
    E_1 = Delta - E_2;
    p_1 = E_1; // massless neutrino approximation
    c_1 = 2*myR->u[3] - 1;
    phi_1 = 2*M_PI*myR->u[4];
    n_from_angles(c_1, phi_1, n_1);
    
    // proton kinematics
    mag_p_f = 0;
    for(int i=0; i<3; i++) {
        p_f[i] = -n_1[i]*p_1 - n_2[i]*p_2;
        mag_p_f += p_f[i]*p_f[i];
    }
    mag_p_f = sqrt(mag_p_f);
}

////////////////////////////////////
////////////////////////////////////


double Gluck_beta_MC::z_VS() const {
    // (3.10)
    if(!omega) return 0;
    double L = SpenceL(2*beta/(1+beta));
    return alpha/M_PI * (3./2.*log(m_p/m_2) + 2*(N/beta-1)*log(2*omega/m_2)
                         +2*N/beta*(1-N) + 2/beta*L - 3./8.);
}

double Gluck_beta_MC::z_H() const {
    // (4.14), noting E0_1/omega = 1/C_S (3.8)
    return alpha/M_PI * (2*(N/beta-1)*(log(1/C_S) + E0_1/(3*E_2) -3./2.)
                         + N/beta * E0_1*E0_1/(12*E_2*E_2));
    
}

void Gluck_beta_MC::vec_rel_n_2(double c, double phi, double* v) const {
    // (5.11)
    const double s = sqrt(1-c*c);
    // (5.9)
    double n_perp[3];
    for(int i=0; i<3; i++) n_perp[i] = np_2[i]*cos(phi) + npp_2[i]*sin(phi);
    // (5.8)
    for(int i=0; i<3; i++) v[i] = n_2[i]*c + n_perp[i]*s;
}

double Gluck_beta_MC::calc_soft() {
    // neutrino direction is relative to electron, so we can use c_1 below
    vec_rel_n_2(c_1, phi_1, n_1);
  
    // (2.12) uncorrected phase space matrix element
    M_0 = 16 * G2_V * zeta * m*m * E0_1 * E_2 * (1 + a * beta * c_1); 
    
    // (3.2) 
    Mtilde = -alpha/M_PI * 16 * G2_V * (1-beta*beta)/beta * N * m*m * E0_1 * E_2 * zeta;
    
    // (3.9)
    M_VS = z_VS() * M_0 + Mtilde;
    
    // weight function
    double W_0VS = beta * E0_1 * E_2 * (M_0 + M_VS);
    evt_w0 =  beta * E0_1 * E_2 * M_0 * evt_w;
    if(W_0VS > Wmax_0VS) Wmax_0VS = W_0VS; 
    sum_W_0VS += W_0VS;
    n_S++;
    
    return W_0VS;
}

double Gluck_beta_MC::calc_hard_brem() {
    
    // (5.5) hard photon energy
    K = omega * pow(C_S,-myR->u[5]);
    
    // (4.9) neutrino energy, corrected for hard photon
    E_1 = Delta - E_2 - K;
    
    // (5.6) gamma direction cosine relative to electron
    c_gamma = (1-(1+beta)*exp(-2*N*myR->u[6]))/beta;
    // (5.7) gamma direction uniform in phi
    phi_gamma = 2*M_PI*myR->u[7];
    // calculate gamma direction in fixed coordinates
    vec_rel_n_2(c_gamma, phi_gamma, n_gamma);
    
    // neutrino direction in hard case is in fixed coordinate system
    // (5.11), (5.12) applied to neutrino direction
    n_from_angles(c_1, phi_1, n_1);
    // or do it relative to electron for consistency... shouldn't matter
    //vec_rel_n_2(c_1, phi_1, n_1);
    
    // (5.13) calculate momentum dot products
    const double p_1_dot_k = E_1 * K * dot3(n_1, n_gamma);
    const double p_2_dot_k = beta * E_2 * K * c_gamma;
    const double p_1_dot_p_2 = beta * E_1 * E_2 * dot3(n_1, n_2);
    // (4.8) four-vector momenta dot
    const double p4_2_dot_k4 = E_2*K - p_2_dot_k;
    
    // (5.3) approximate distribution function
    const double g = beta*E_2/(2*N*p4_2_dot_k4);
    
    // (4.7)
    const double Psq = 1./K/K + m_2*m_2/p4_2_dot_k4/p4_2_dot_k4 - 2*E_2/K/p4_2_dot_k4;
    // (4.5)
    const double H_0 = E_1*(-(E_2+K)*Psq + K/p4_2_dot_k4);
    // (4.6)
    const double H_1 = ( p_1_dot_p_2 * ( -Psq + 1./p4_2_dot_k4 )
                        + p_1_dot_k * ((E_2+K)/K -m_2*m_2/p4_2_dot_k4) / p4_2_dot_k4);
    // (4.3)
    const double esq = 4*M_PI*alpha;
    // (4.4)
    M_BR = 16*G2_V*zeta*m*m*esq*(H_0 + a*H_1);
    
    // (5.14)
    double w = (K * beta * E_1 * E_2 * M_BR)/(pow(2,13)*pow(M_PI,8)*m*m*g);
    if(w > w_max) w_max = w;
    sum_w += w;
    n_H++;
    
    return w;
}

double Gluck_beta_MC::recalc_Sirlin_g_a2pi(double E_e) {
    E_2 = E_e;
    E0_1 = Delta - E_2;
    calc_beta_N();
    // (4.15)
    return z_VS() - alpha/M_PI * N * (1-beta*beta)/beta + z_H();
}

void Gluck_beta_MC::calc_beta_N() {
    // (2.10)
    beta = sqrt(1-m_2*m_2/(E_2*E_2));
    if(!(beta==beta)) beta = 0;
    // (3.3)
    N = 0.5*log((1+beta)/(1-beta));
    // (3.8) soft brem cutoff
    omega = C_S * E0_1;
}

void Gluck_beta_MC::propose_kinematics() {
    
    K = 0;
    evt_w = 1;
    
    E_2 = m_2 + (Delta-m_2)*myR->u[0];  // (5.4) electron energy
    p_2 = sqrt(E_2*E_2 - m_2*m_2);      // electron momentum magnitude
    
    // electron cos theta to axis... possibly pre-restricted range
    c_2_min = -1;
    if(pt2_max && p_2 > pt2_max)
        c_2_min = sqrt(1.-pt2_max*pt2_max/(p_2*p_2));
    c_2_wt = (1-c_2_min)/2;
    c_2 = c_2_min + (1-c_2_min)*myR->u[2]; //c_2 = -1 + 2*myR->u[2];
    // electron azimuthal angle
    phi_2 = 2*M_PI*myR->u[4];
    
    // (2.10) neutrino energy by energy conservation (tweak later for photon emission)
    E_1 = E0_1 = Delta - E_2;
       
    // (5.7) isotropic neutrino direction (rel. to electron in "soft" case; world coordinates in "hard")
    c_1 = 2*myR->u[1] - 1;
    phi_1 = 2*M_PI*myR->u[3];
    
    
    calc_beta_N();
    calc_n_2();
}

void Gluck_beta_MC::calc_n_2() {
    // (5.11)
    const double s_2 = sqrt(1-c_2*c_2);
    // (5.12)
    n_2[0] = s_2*cos(phi_2); n_2[1] = s_2*sin(phi_2); n_2[2] = c_2;
    // (5.10)
    np_2[0] = -sin(phi_2);      np_2[1] = cos(phi_2);           np_2[2] = 0;
    npp_2[0] = -c_2*cos(phi_2); npp_2[1] = -c_2*sin(phi_2);     npp_2[2] = s_2;
}

void Gluck_beta_MC::gen_evt_weighted() {
    assert(myR);
    myR->next();
    
    if(P_H.q < myR->u[4]) {
        myR->u[4] = (myR->u[4]-P_H.q)/(1-P_H.q);
        propose_kinematics();
        
        evt_w *= calc_soft()/Wavg_0VS * P_H.np_wt();
        evt_w0 /= Wavg_0VS;
    } else {
        myR->u[4] /= P_H.q;
        propose_kinematics();
       
        evt_w *= calc_hard_brem()/w_avg * P_H.p_wt();
        evt_w0 = 0;
    }
    calc_proton();
}

void Gluck_beta_MC::calc_rho() {
    
    printf("Calculating correction rates:\n");
    
    // numerical integration over electron energy E2 by Simpson's Rule
    const int npts = 4001;
    rho_0 = rho_VS = rho_H = 0;
    const double C = G2_V * zeta / (2*pow(M_PI,3));
    for(int i=0; i<=npts; i++) {
        
        E_2 = m_2 + i*(Delta-m_2)/npts;
        E0_1 = Delta - E_2;
        calc_beta_N();
        
        if(!beta) continue;
        //printf("E_2 = %g, beta = %g, z_VS = %g, z_H = %g\n", E_2, beta, z_VS(), z_H());
        
        const int scoeff = (i==0 || i==npts)? 1: (i%2)? 4:2; // Simpson's Rule integration weight
        // (5.19)
        const double w0 = C * beta * E0_1*E0_1 * E_2*E_2;
        // (5.18)
        rho_0 += scoeff * w0;
        // (5.20)
        rho_VS += scoeff * w0 * (z_VS() - alpha/M_PI * N * (1-beta*beta)/beta);
        // (4.13)
        rho_H += scoeff * w0 * z_H(); 
    }
    const double nrm = (Delta-m_2)/npts/3.;
    rho_0 *= nrm;
    rho_VS *= nrm;
    rho_H *= nrm;
    
    rho_0VS = rho_0 + rho_VS;
    
    // (5.21)
    P_H.p = P_H.q = rho_H/(rho_H + rho_0VS);
    
    // (5.15)
    V_g = -32*pow(M_PI,3)*(Delta-m_2)*log(C_S);
    
    w_avg = rho_H / V_g;
    
    // (6.1)
    r_rho = 100*(rho_VS+rho_H)/rho_0;
    r_H = 100*P_H.p;
    
    printf("\trho_0 = %g, rho_VS = %g, rho_H = %g => P_H = %g\n", rho_0, rho_VS, rho_H, P_H.p);
    printf("\tr_rho = %g (Gluck: 1.503),\tr_H = %g (Gluck: 0.847)\n", r_rho, r_H);
}

void Gluck_beta_MC::test_calc_P_H(size_t n_sim) {
    
    // (5.16), (5.17) MC calculation of rho_H
    double t_rho_H = 0;
    double sw2 = 0;
    printf("Calculating P_H using %zu points... ", n_sim);
    for(size_t i=0; i<n_sim; i++) {
        if(!(i%(n_sim/20))) { printf("*"); fflush(stdout); }
        
        myR->next();
        
        propose_kinematics();
        double w = calc_hard_brem();
        t_rho_H +=  w;
        sw2 += w*w;
        
        // do this calculation to build up statistics on Wmax_0VS
        propose_kinematics();
        calc_soft();
    }
    printf(" Done.\n");
    double dt_rho_H = sqrt(sw2 - t_rho_H*t_rho_H/n_sim)*V_g/n_sim;
    t_rho_H *= V_g/n_sim;
    
    // (5.21)
    double t_P_H = t_rho_H/(t_rho_H + rho_0VS);
    double dt_P_H = dt_rho_H/(t_rho_H + rho_0VS);
    printf("\tMC rho_H = %g +/- %g, P_H = %g\n", t_rho_H, dt_rho_H, t_P_H);
    printf("\tMC r_H = %g +- %g;\t\tV_g = %g\n", 100*t_P_H, 100*dt_P_H, V_g);
    
    showEffic();
}

void Gluck_beta_MC::showEffic() {
    if(n_S) {
        Wavg_0VS = sum_W_0VS/n_S;       // (5.23)
        E_0VS = 100*Wavg_0VS/Wmax_0VS;  // (5.22)
        printf("\tWmax_0VS = %g;\tWavg_0VS = %g;\tE_0VS = %.1f%% (Gluck: 56%%)\n", Wmax_0VS, Wavg_0VS, E_0VS);
    }
    if(n_H) {
        w_avg = sum_w/n_H;     // (5.23)
        E_H = 100*w_avg/w_max; // (5.22)
        printf("\tw_max = %g;\tw_avg = %g;\tE_H = %.1f%% (Gluck: 28%%)\n", w_max, w_avg, E_H);
    }
}

double Gluck_beta_MC::rwm_cxn() const {
    return B59_rwm_cxn(E_2, dot3(n_1, n_2));
}

double B59_rwm_cxn(double E, double cos_thn) {
    // Expecting lambda = |lambda| sign convention
    
    const double mu = delta_mu;
    const double Delta = m_n-m_p;
    const double M = m_n;
    
    const double beta = sqrt(1-m_e*m_e/(E*E));
    const double x = 1 + 3*lambda*lambda;
    const double c1 = 1 + lambda*lambda;
    const double c2 = 1 - lambda*lambda;
    const double lpm = lambda + mu;
    const double c4 = 1 + lambda*lambda + 2*lambda*mu;
    
    // note, compared to Bilenkii A(E), we've already factored out the spectrum shape,
    // leaving 1+O(E/M) angle-independent shape correction term
    double A = ( 1 + (3 + 4*lambda*mu/x)*E/M
                - c4/x * m_e*m_e/(M*E)
                - 2*lambda*lpm*Delta/M/x);
    // angle-dependent terms
    double a0 = c2/x;
    double a = a0 + ( 4*lambda*c1*lpm*Delta/M
                     +c2*c4*m_e*m_e/(M*E)
                     -(8*lambda*c1*mu + 3*x*x)*E/M )/(x*x);
    double b = -3 * a0 * E/M;
    
    // divide out 1+a0*beta*cos_thn zeroth-order angular dependence to produce correction factor
    return A*(1 + beta*a*cos_thn + beta*beta*b*cos_thn*cos_thn)/(1+beta*a0*cos_thn);
}

double GM78_radiative_cxn(double E, double cos_thn) {
    const double E_m = delta_mn_mp;
    double beta = sqrt(1-m_e*m_e/(E*E));
    double athb = atanh(beta);
    double c0 = athb/beta;
    double c1 = 3./2.*log(m_p/m_e) - 3./8. + 2./beta*SpenceL(2*beta/(1+beta));
    double c2 = log(2*(E_m-E)/m_e);
    
    
    double phth1 = ( c1 + 2*(c0-1)*( (E_m-E)/(3*E) - 3./2. + c2 )  
                    + c0/2.*(2*(1+beta*beta) + (E_m-E)*(E_m-E)/(6*E*E) - 4*athb) ) * alpha/M_PI;
                    
    double phth2 = ( c1 + (c0-1)*((E_m-E)*(E_m-E)/(12*beta*beta*E*E) + 2*(E_m-E)/(3*E*beta*beta) + 2*c2 - 3)
                    -2*c0*(athb-1) ) * alpha/M_PI;
                    
    //double r0 = (1 + 3*lambda*lambda + beta*cos_thn*(1 - lambda*lambda));
     
    double r0 = (1 + 3*lambda*lambda)*(1.+Wilkinson_g_a2pi(E/m_e));
    return (1 + phth1 + 3*lambda*lambda*(1+phth1)
            + beta*cos_thn*(1 + phth2 - lambda*lambda*(1 + phth2)) )/r0;
}

double Gluck93_F_C(double E_2, double b) {
    // (2.5)
    const double R = 0.01/(4*m_e);
    return 1 + alpha*M_PI/b + alpha*alpha*(11./4. - gamma_euler - log(2*b*E_2*R) + M_PI*M_PI/(3*b*b));
}

double Gluck93_Distribution::calc_W_0C(double E_2, double E_f, double c_f) {
    // (3.3) neutrino energy and proton-recoil-corrected endpoints
    double E_1 = m_i - E_2 - E_f;
    double E_2m = Delta - (Delta*Delta - m_2*m_2)/(2*m_i);
    double E_1m = Delta - (Delta*Delta + m_2*m_2)/(2*m_i);
    double E_fm = m_f + (Delta*Delta - m_2*m_2)/(2*m_i);
    // (3.2) recoil-order corrected spectrum without Fermi function
    double D_V = E_2*(E_2m-E_2) + E_1*(E_1m-E_1) - m_f*(E_fm-E_f);
    double D_A = E_2*(E_2m-E_2) + E_1*(E_1m-E_1) + m_f*(E_fm-E_f);
    double D_I = 2*(E_2*(E_2m-E_2)-E_1*(E_1m-E_1));
    // note negative lambda sign convention!
    double W_0 = m_i * G2_V / (4*M_PI*M_PI*M_PI) * (D_V + lambda*lambda*D_A - fabs(lambda)*(1+2*kappa)*D_I);
    
    double p_f = sqrt(E_f*E_f - m_f*m_f);
    double v_f = p_f/E_f;
    //double c_f = +1; // TODO cos electron/proton angle = p_2.p_f / |p_2||p_f|
    double beta_r = fabs(beta-(1-beta*beta)*v_f*c_f); // (3.4) electron-proton relative velocity
    // (2.5)
    double Fhat_C = Gluck93_F_C(E_2, beta_r);
    
    // (3.1)
    return W_0C = W_0 * Fhat_C;
}

double Gluck93_Distribution::calc_Wenu_0Ca(double E_2, double c) {
    calc_Wenu_0(E_2, c); // gets p_2, beta
    
    // (4.3)
    double d = m_i - E_2;
    double E_fc = (d*d + p_2*p_2 + m_f*m_f + 2*d*p_2*c)/(2*(d+p_2*c));
    calc_W_0C(E_2, E_fc);
    
    // (4.4)
    dEfc_dc = p_2*(E_2m - E_2)/m_i * (1 + 2*E_2/m_i*(1-beta*c));
    
    
    double x = (E_2-m_2)/(E_2m-m_2);    // (2.6)
    double r_enu = Gluck93_r_enu(x,c);  // from parametrized fit to Table V
    double r_e = 100*Wilkinson_g_a2pi(E_2/m_2);
    // (4.2)
    return Wenu_0Ca = W_0C * dEfc_dc * (1 + 0.01*r_e + 0.01*r_enu);
}

double Gluck93_Distribution::calc_Wenu_0(double E_2, double c) {
    // (2.5)
    p_2 = sqrt(E_2*E_2 - m_2*m_2);
    beta = p_2/E_2;
    // (4.6)
    return Wenu_0 = G2_V*(1+3*lambda*lambda)/(4*M_PI*M_PI*M_PI)*p_2*E_2*pow(E_2m-E_2,2)*(1+a0*beta*c);
}