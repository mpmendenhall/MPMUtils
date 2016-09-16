/// \file PointCloudHistogram.hh Multi-dimensional histogram binned around point cloud locations
#include <TKDTree.h>
#include <TH1.h>
#include <TGraph.h>
#include <cassert>
#include <map>
#include <vector>

using std::vector;
using std::map;

/// wrapper for kd-tree and point lists
class KDTreeSet {
public:
    /// constructor
    KDTreeSet(unsigned int n): ndim(n), fData(n), T(nullptr) {}

    const unsigned int ndim;            ///< number of dimensions
    vector< vector<float> > fData;      ///< coordinate points
    TKDTree<int,float>* T;              ///< kd-tree of data points

    /// build kd-tree
    void finalize();
    /// get number of points
    unsigned int nPts() const { return fData[0].size(); }

    /// add points filling specified range
    void fillPointRange(unsigned int npts, const float* xlo, const float* xhi, const float* dens = nullptr);
};


/// Multi-dimensional sparse histogram with bins defined by Voronoi diagram of supplied kd-tree
class PointCloudHistogram {
public:
    /// constructor
    PointCloudHistogram(KDTreeSet* T): myTree(T) { assert(myTree->T); }

    /// add value to nearest point
    void Fill(const float* x, float v = 1.0);

    /// project onto given vector, filling results into supplied TGraph
    void project(const float* v, TGraph& g) const;

    /// project onto given vector, filling given TH1
    void project(const float* v, TH1& h) const;

    /// get indexed point location
    void getPoint(unsigned int i, float* x) const;

    /// get number of dimensions
    unsigned int getNdim() const { return myTree->ndim; }

protected:
    KDTreeSet* myTree;          ///< kd-tree defining binning
    map<int, float> bins;       ///< counts in each bin
};
