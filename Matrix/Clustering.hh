/// @file Clustering.hh K-means and Expectation-maximization clustering routines
// This file was produced under the employ of the United States Government,
// and is consequently in the PUBLIC DOMAIN, free from all provisions of
// US Copyright Law (per USC Title 17, Section 105).
//
// -- Michael P. Mendenhall, 2015

#ifndef CLUSTERING_HH
#define CLUSTERING_HH

#include "VarMat.hh"

/// Class for k-means segmentation
class KMeansCalculator {
public:
    /// Constructor
    explicit KMeansCalculator(unsigned int m): nmeans(m), class_counts(m), class_means(m) { }

    /// full means and reclassification step
    unsigned int kMeans_step();
    /// classification component of step
    unsigned int classify();
    /// means calculation component of step
    void calcMeans();
    /// count points in each class
    void calcClassCounts();

    /// calculate variance (mean squared deviation) in each class
    vector<double> calcVariance() const;

    const unsigned int nmeans;                          ///< number of means

    vector< VarVec<double> > points;                    ///< points being classified
    vector<unsigned int> classification;                ///< k-means classification for each point

    VarVec<unsigned int> class_counts;                  ///< counts in each class
    vector< VarVec<double> > class_means;               ///< means for each class
};

/// Class for Expectation-maximization clustering
class EMClusterer {
public:
    /// Constructor
    EMClusterer(size_t nclust, unsigned int ndim): m(nclust), k(ndim) { }

    /// add data point
    void addPoint(const VarVec<double>& v) { assert(v.size()==k); points.push_back(v); n=0; }

    /// initialize from k-means initial guess
    void initFromKMeans(const KMeansCalculator& K);

    /// perform step
    void step() { if(!n) init(); E_step(); M_step(); }

    /// log probability of point being in cluster j
    double logprob(const VarVec<double>& x, unsigned int j) const;
    /// probability of point being in cluster j
    double prob(const VarVec<double>& x, unsigned int j) const  { return exp(logprob(x,j)); }

    /// log likelihood of current solution
    double logL() const;

    /// Array initialization
    void init();
    /// "Expectation" calculation step
    void E_step();
    /// "Maximization" caclualtion step
    void M_step();

    vector< VarVec<double> > points;    ///< points being classified
    vector<unsigned int> classification;///< points' most likely class

    const size_t m;                     ///< number of clusters
    size_t n = 0;                       ///< number of points
    const unsigned int k;               ///< number of dimensions
    vector< VarVec<double> > mu;        ///< means for each class
    vector< VarMat<double> > Sigma;     ///< covariance matrix for each class
    vector< VarMat<double> > iSigma;    ///< covariance matrix inverse for each class
    vector<double> detSigma;            ///< covariance matrix determinant for each class
    VarVec<double> tau;                 ///< proportion in each cluster
    VarMat<double> f;                   ///< probabilities for each point's assignment
    VarMat<double> T;                   ///< conditional distribution
};

#endif
