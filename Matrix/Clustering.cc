/// \file Clustering.cc
// This file was produced under the employ of the United States Government,
// and is consequently in the PUBLIC DOMAIN, free from all provisions of
// US Copyright Law (per USC Title 17, Section 105).
// 
// -- Michael P. Mendenhall, 2015

#include "Clustering.hh"
#include <cfloat>
#include <cassert>

void KMeansCalculator::calcClassCounts() {
    printf("\tClassification counts:");
    class_counts = VarVec<unsigned int>(nmeans);
    for(auto it = classification.begin(); it != classification.end(); it++) class_counts[*it]++;
    for(unsigned int j=0; j<nmeans; j++) printf("\t[%u] %u", j, class_counts[j]);
    printf("\n");
}

void KMeansCalculator::calcMeans() {
    class_means = vector< VarVec<double> >(nmeans, VarVec<double>(points[0].size()));
    for(unsigned int i=0; i<points.size(); i++) {
        unsigned int c = classification[i];
        if(c >= nmeans) continue;
        class_means[c] += points[i];
    }
    for(unsigned int c=0; c<nmeans; c++) class_means[c] /= class_counts[c];
}

unsigned int KMeansCalculator::classify() {
    vector<double> mindist(points.size(), DBL_MAX);
    auto oldclass = classification;
    unsigned int nreclassified = 0;
    for(unsigned int i=0; i<points.size(); i++) {
        for(unsigned int c=0; c<nmeans; c++) {
            double d = (points[i] - class_means[c]).mag2();
            if(d < mindist[i]) { mindist[i] = d; classification[i] = c; }
        }
        if(classification[i] != oldclass[i]) nreclassified++;
    }
    calcClassCounts();
    return nreclassified;
}

unsigned int KMeansCalculator::kMeans_step() {
    printf("k-means step...");
    calcMeans();
    unsigned int nreclassified = classify();
    printf(" %u reclassified.\n",nreclassified);
    return nreclassified;
}

vector<double> KMeansCalculator::calcVariance() const {
    vector<double> v(nmeans);
    for(size_t i=0; i<points.size(); i++) v[classification[i]] += (points[i]-class_means[classification[i]]).mag2();
    for(size_t j=0; j<nmeans; j++) v[j] /= class_counts[j];
    return v;
}

////////////////////
////////////////////
////////////////////

void EMClusterer::init() {
    n = points.size();
    f = VarMat<double>(m,n);
    T = VarMat<double>(m,n);
    mu.resize(m);
    iSigma.resize(m);
    detSigma.resize(m);
}

double EMClusterer::logprob(const VarVec<double>& x, unsigned int j) const {
    assert(j<m);
    auto xm = x - mu[j];
    return -xm.dot(iSigma[j]*xm)/2 - 0.5*(log(detSigma[j]) + k*log(2*M_PI));
}

void EMClusterer::initFromKMeans(const KMeansCalculator& K) {
    points = K.points;
    n = points.size();
    classification.resize(n);
    mu = K.class_means;
    tau = convertType<unsigned int, double>(K.class_counts)/n;
    
    iSigma.clear();
    Sigma.clear();
    detSigma.clear();
    auto sig2 = K.calcVariance();
    for(size_t j=0; j<m; j++) {
        Sigma.push_back(VarMat<double>::identity(k, sig2[j], 0));
        iSigma.push_back(VarMat<double>::identity(k, 1./sig2[j], 0));
        detSigma.push_back(1./pow(sig2[j],k));
    }
    
    f = VarMat<double>(n,m);
    T = VarMat<double>(m,n);
}

double EMClusterer::logL() const {
    double ll = 0;
    for(unsigned int i=0; i<n; i++) {
        unsigned int j = classification[i];
        ll += log(tau[j]) + logprob(points[i],j);
    }
    return ll;
}

void EMClusterer::E_step() {
    for(size_t i=0; i<n; i++) {
        double denom = 0;
        for(size_t j=0; j<m; j++) {
            f(i,j) = prob(points[i],j);
            denom +=  tau[j]*f(i,j);
        }
        assert(denom);
        double bestprob = -1;
        for(size_t j=0; j<m; j++) {
            T(j,i) =  tau[j]*f(i,j)/denom;
            if(T(j,i) > bestprob) { bestprob = T(j,i); classification[i] = j; }
        }
    }
}

void EMClusterer::M_step() {
    tau = T.getRowSum()/n;
    for(size_t j=0; j<m; j++) {
        mu[j].zero();
        for(size_t i=0; i<n; i++) mu[j] += points[i]*T(j,i);
        mu[j] /= n*tau[j];
        
        Sigma[j].zero();
        for(size_t i=0; i<n; i++) {
            auto xu = points[i]-mu[j];
            Sigma[j] += outer(xu,xu)*T(j,i);
        }
        Sigma[j] /= n*tau[j];
        
        iSigma[j] = Sigma[j]; iSigma[j].invert();
        detSigma[j] = Sigma[j].det();
    }
}
