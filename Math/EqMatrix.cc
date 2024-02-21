/// @file

#include "EqMatrix.hh"
#include <algorithm>
#include <TMatrixD.h>
#include <TVectorD.h>
#include <TDecompQRH.h>

void EqMatrix::addDiff(int i, int j, double x, double dx) {
    lineq_t e;
    e.coeffs.emplace_back(i, 1.);
    e.coeffs.emplace_back(j,-1.);
    e.x = x;
    e.dx = dx;
    dpts.push_back(e);
}

void EqMatrix::addSum(int i, int j, double x, double dx) {
    lineq_t e;
    e.coeffs.emplace_back(i, 1.);
    e.coeffs.emplace_back(j, 1.);
    e.x = x;
    e.dx = dx;
    dpts.push_back(e);
}

void EqMatrix::lineq_t::display() const {
    for(auto& p: coeffs) printf("%+g * [%i] ", p.second, p.first);
    printf("= %g +- %g\n", x, dx);
};

void EqMatrix::index_vars() {
    indices.clear();
    for(auto& d: dpts) for(auto& c: d.coeffs) indices.push_back(c.first);
    std::sort(indices.begin(), indices.end());
    indices.erase(std::unique(indices.begin(), indices.end()), indices.end());

    to_idx.clear();
    int i = 0;
    for(auto j: indices) to_idx[j] = i++;
}

void EqMatrix::calculate(bool doErrs) {
    index_vars();
    auto N = indices.size();
    auto K = dpts.size();
    TMatrixD M(K, N);
    TVectorD v(K);

    size_t i = 0;
    sumw = 0;
    for(auto& p: dpts) {
        p.dx = 1./(p.dx * p.dx);
        sumw += p.dx;
        for(auto& c: p.coeffs) {
            v[i] = p.x*p.dx;
            M(i, to_idx.at(c.first)) += c.second*p.dx;
        }
        i++;
    }
    printf("Decomposing %zu x %zu equations matrix\n", K, N);

    TDecompQRH D(M);
    D.Solve(v);

    soln.resize(N);
    for(i=0; i<N; i++) {
        soln[i].var = indices[i];
        soln[i].x = v[i];
    }

    if(doErrs) {
        printf("Calculating error estimate...");
        auto Mi = D.Invert(); // pseudo-inverse N x K
        TVectorD vv(N);
        for(i=0; i<N; i++) {
            for(size_t j=0; j<K; j++) soln[i].dx += Mi(i,j) * Mi(i,j) * dpts[j].dx;
            soln[i].dx = sqrt(soln[i].dx);
            vv[i] = v[i];
        }
        auto r = M*vv;
        rms = 0;
        for(i=0; i<K; i++) rms += pow(dpts[i].x - r[i]/dpts[i].dx, 2) * dpts[i].dx;
        rms /= sumw;
        rms = sqrt(rms);
        printf(" RMS %g\n", rms);
    }
}
