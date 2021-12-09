/// \file FresnelEqs.cc

#include "FresnelEqs.hh"
#include <stdexcept>

void s_IndexChange::operator+=(const s_IndexChange& X) {
    if(X.ccrit > bcrit) { // new forward-critical-angle limitation
        ccrit = cth_tx(X.ccrit, 1./r);
        bcrit = X.bcrit; // = 0
    } else if(bcrit > X.ccrit) { // new backward-critical-angle limitation
        bcrit = cth_tx(bcrit, X.r);
        //ccrit = ccrit; // = 0
    } else {
        bcrit = X.bcrit;
    }
    r *= X.r;
}

void s_IndexChange::display(bool end_l) const {
    printf("[r = %4.2f: %4.2f|%4.2f]", r, ccrit, bcrit);
    if(end_l) printf("\n");
}

//----------------------

void s_RxTx::setFresnel(double c0, double c1, double r, double A) {
    Rx = (r*c0 - c1)/(r*c0 + c1);
    Rx *= Rx;
    Tx = (1 - Rx)*A;
    Rx *= A*A;
}

/*
 * 1 -->| -->U|
 * R <--|V<-- |--> T
 *      |     |
 *    R0,T0 R1,T1
 *
 * R = R0 + T0*V
 * T = T1*U
 *
 * U = T0 + V*R0
 * V = R1*U
 *
 * => U = T0/(1-R0 R1), V = ... etc
 */

void s_RxTx::operator+=(const s_RxTx& X) {
    if(Rx * X.Rx == 1) {
        Rx = 1;
        Tx = 0;
        return;
    }
    double U = Tx/(1 - Rx*X.Rx);
    Rx += Tx * X.Rx * U;
    Tx = X.Tx * U;
}

/*
 *      | -->V|
 *      |U<-- |
 * A <--|     |--> B
 *      |I1-->|
 *      |<--I0|
 *    R0,T0 R1,T1
 *
 * U = I0 + R1*V
 * V = I1 + R0*U
 *
 * A = T0*(I0+U)
 * B = T1*(I1+V)
 */

pair<double,double> s_RxTx::between(const s_RxTx& S0, const s_RxTx& S1, double I0, double I1) {
    if(S0.Rx * S1.Rx == 1) return {0,0};
    double k = 1./(1 - S0.Rx * S1.Rx);
    return {S0.Tx * (I0 + S1.Rx*I1) * k,
            S1.Tx * (I0*S0.Rx + I1) * k };
}

//----------------------

void s_Fresnel_Rx::set_ci_single(double _ci) {
    ci = _ci;
    ct = 1-r*r*(1-ci*ci);
    double A = A0 < 1? (ci > 0? pow(A0, 1./ci): 0.) : 1.;

    if(ct <= 0) { // TIR'd
        ct = 0;
        X_p = s_RxTx::mirror(A*A);
        X_s = s_RxTx::mirror(A*A);
        return;
    }

    ct = sqrt(ct);
    X_s.setFresnel(ci, ct, r, A);
    X_p.setFresnel(ct, ci, r, A);
}

void s_Fresnel_Rx::display() const {
    printf("Fresnel ");
    s_IndexChange::display(false);
    printf(": cos th %4.2f -> %4.2f, S ", ci, ct);
    X_s.display(", P ");
    X_p.display();
}

void s_Fresnel_Rx::operator+=(const s_Fresnel_Rx& R) {
    if(ct != R.ci) throw std::logic_error("mismatched propagation angles");
    (s_IndexChange&)(*this) += R;
    ct = R.ct;
    X_s += R.X_s;
    X_p += R.X_p;
}

//----------------------

void FresnelStack::set_cth0(double cth) {
    (s_Fresnel_Rx&)(*this) = s_Fresnel_Rx(1,cth);

    for(auto& F: *this) {
        F.set_ci_single(ct);
        (*this) += F;
    }
}

void FresnelStack::display() const {
    s_Fresnel_Rx::display();
    for(auto& F: *this) { printf("\t"); F.display(); }
}
