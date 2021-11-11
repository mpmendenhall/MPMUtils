/// \file PointCloudHistogram.hh Multi-dimensional histogram binned around point cloud locations
// Michael P. Mendenhall

#include <TKDTree.h>
#include <TH1.h>
#include <TGraph.h>
#include <vector>
using std::vector;
#include <stdexcept>

/// wrapper for kd-tree and point lists
class KDTreeSet: protected vector<vector<float>> {
public:
    typedef vector<vector<float>> super_t;

    /// constructor
    explicit KDTreeSet(unsigned int n): super_t(n) { }
    /// destructor
    ~KDTreeSet() { clearTree(); }

    TKDTree<int,float>* T = nullptr;    ///< kd-tree of data points

    /// get number of dimensions
    size_t nDim() const { return size(); }
    /// get number of points
    size_t nPts() const { return at(0).size(); }
    /// element access
    float operator()(size_t a, size_t i) const { return (*this)[a][i]; }
    /// project element onto provided vector
    float project(size_t i, const float* v) const;

    /// extract point into nDim()-dimensional array
    void getPoint(unsigned int i, float* x) const { for(auto& v: *this) *(x++) = v[i]; }

    /// add point from nDim()-dimensional array
    void addPoint(float* x) { for(auto& v: *this) v.push_back(*(x++)); }
    /// remove elements at sorted index list
    void remove_points(const vector<size_t>& vidx);

    /// build kd-tree
    void finalize();
    /// clear kd tree
    void clearTree() { delete T; T = nullptr; }
};


/// Multi-dimensional histogram with bins defined by Voronoi diagram of supplied kd-tree
class PointCloudHistogram: protected vector<float> {
public:
    typedef vector<float> super_t;

    /// constructor
    explicit PointCloudHistogram(KDTreeSet& T): super_t(T.nPts()), myTree(&T) {
        if(!T.T) throw std::logic_error("PointCloudHistogram requires constructed KDTree");
    }

    /// add value to nearest point
    void Fill(const float* x, float v = 1.0);

    /// project onto given vector, filling results into supplied TGraph
    void project(const float* v, TGraph& g) const;

    /// project onto given vector, filling given TH1
    void project(const float* v, TH1& h) const;

    /// number of bins
    using super_t::size;
    /// iteration helper
    using super_t::begin;
    /// iteration helper
    using super_t::end;
    /// bin contents access
    using super_t::at;

protected:
    KDTreeSet* myTree;  ///< kd-tree defining binning
};
