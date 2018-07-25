/// \file BoxTree.hh Dividing edges for KD Tree

#include <vector>
using std::vector;
#include <map>
using std::map;
#include <cfloat>       // for DBL_MAX
#include <stdlib.h>     // for size_t
#include <functional>   // for std::function

/// Dividing edges for KD Tree
class BoxTreeNode {
public:
    /// Constructor
    BoxTreeNode() { }
    /// Destructor
    ~BoxTreeNode() { delete cLo; delete cHi; }

    /// iterator over tree nodes
    template<typename ptr_t>
    class iterator_t {
    public:
        /// constructor ending at node
        iterator_t(ptr_t n = nullptr): N(n) { }
        /// dereference to get node
        ptr_t operator*() { return N; }
        /// crawl to next node
        iterator_t& operator++();
        /// check if iterators unequal
        bool operator!=(const iterator_t& rhs) const { return N != rhs.N || hiSide != rhs.hiSide; }
    protected:
        ptr_t N;                    ///< current node
        vector<bool> hiSide;        ///< chain of hi/low side positions
        /// crawl to lowest low node
        void descend_low();
        friend class BoxTreeNode;
    };

    typedef iterator_t<BoxTreeNode*> iterator;
    typedef iterator_t<const BoxTreeNode*> const_iterator;

    /// iterator starting bottom-up crawl through all sub-nodes
    iterator begin();
    /// end iterator (same for all trees!)
    iterator end() { return iterator(nullptr); }
    /// iterator starting bottom-up crawl through all sub-nodes
    const_iterator begin() const;
    /// end iterator (same for all trees!)
    const_iterator end() const { return const_iterator(nullptr); }

    // node information queries

    /// get axis
    inline int getAxis() const { return axis; }
    /// get split position
    inline double getSplit() const { return split; }
    /// whether this is terminal leaf node
    inline bool isLeaf() const { return (!cLo) || (!cHi); }
    /// get low-side subnode
    inline const BoxTreeNode* getLo() const { return cLo; }
    /// get high-side subnode
    inline const BoxTreeNode* getHi() const { return cHi; }
    /// get low-side subnode
    inline BoxTreeNode* getLo() { return cLo; }
    /// get high-side subnode
    inline BoxTreeNode* getHi() { return cHi; }
    /// whether this is the low-side split
    inline bool isLo() const { return parent? this == parent->cLo: false; }
    /// whether this is the high-side split
    inline bool isHi() const { return parent? this == parent->cHi: false; }
    /// count number of nodes
    size_t size() const;
    /// count number of leaf nodes
    size_t nLeaves() const;
    /// count number of axis splits
    size_t nSplits(int a) const;
    /// low boundary on given axis
    double bLo(int a) const;
    /// high boudary on given axis
    double bHi(int a) const;
    /// whether range is bounded below on given axis
    inline bool isBoundedLo(int a) const { return bLo(a) > -DBL_MAX; }
    /// whether range is bounded above on given axis
    inline bool isBoundedHi(int a) const { return bHi(a) < DBL_MAX; }
    /// span along axis
    inline double span(int a) const { return bHi(a) - bLo(a); }
    /// product of spans along N_DIM axes
    inline double volume(int N) const { double V = 1; for(int a=0; a<N; a++) V *= span(a); return V; }
    /// center along axis
    inline double center(int a) const { return 0.5*(bHi(a) + bLo(a)); }
    /// whether value is contained along axis
    inline bool contains(double x, int a) const { return bLo(a) <= x && x < bHi(a); }
    /// whether value is contained in 0...nAxes axes
    inline bool contains(double* x, int nAxes) const { for(int a=0; a<nAxes; a++) if(!contains(x[a],a)) return false; return true; }
    /// whether axis is bounded above and below
    inline bool isBounded(int a) const { return bLo(a) > -DBL_MAX && bHi(a) < DBL_MAX; }
    /// count node depth from top
    unsigned int depth() const { return parent? parent->depth()+1 : 0; }
    /// determine depth of deepest child node
    unsigned int maxdepth() const { return std::max(cLo? cLo->maxdepth()+1 : 0, cHi? cHi->maxdepth()+1 : 0); }

    // navigation

    /// get top of tree
    const BoxTreeNode* getTop() const { return parent? parent->getTop() : this; }
    /// locate node containing point
    const BoxTreeNode* locate(const double* d) const;
    /// locate node containing center of other node
    const BoxTreeNode* locateCenter(const BoxTreeNode& N, map<int,double>& cs) const;
    /// find nodes matching criteria, stopping at matching nodes
    void findNodes(vector<const BoxTreeNode*>& v, std::function<bool(const BoxTreeNode&)> f) const;
    /// find leaf nodes, given acceptance function for all nodes
    void findLeafNodes(vector<const BoxTreeNode*>& v, std::function<bool(const BoxTreeNode&)> f) const;

    // modification

    /// Create clone
    BoxTreeNode* clone() const;
    /// split node along axis, returning splitting node
    BoxTreeNode* splitNode(int a, double s);
    /// recursively split by another tree, returning top split node
    BoxTreeNode* splitNode(const BoxTreeNode* N);
    /// create by splits and return sub-node with given axis bounds
    BoxTreeNode* bound(int a, double s0, double s1);
    /// project out axis, collapsing nodes split on this axis into one
    BoxTreeNode* projectOut(int a);


protected:
    int axis;                       ///< axis number for split
    double split;                   ///< split position
    BoxTreeNode* parent = nullptr;  ///< parent node; NULL at top level
    BoxTreeNode* cLo = nullptr;     ///< low-side child; NULL at bottom level
    BoxTreeNode* cHi = nullptr;     ///< high-side child; NULL at bottom level

    /// set children to point to this as parent
    void adopt();

    /// split on dividing line; return low and high side nodes, and whether this node is used for one of those
    bool _split(int a, double s, BoxTreeNode*& newLo, BoxTreeNode*& newHi);
};

/////////////////////////
/////////////////////////

/// Helper class to build KD tree from float* arrays
class KDBuilder {
public:
    /// Constructor
    KDBuilder(int N): N_DIM(N) { }

    const int N_DIM;                ///< number of dimensions
    unsigned int min_divide_points = 20;    ///< minimum number of points to continue subdividing
    bool closeBounds = false;       ///< whether to bound remaining open edges around last points
    int snapgrid = 0;               ///< if >0, snap to 2^snapgrid -- 2^(snapgrid+1) points
    bool smartDivide = false;       ///< attempt smarter division point finding

    /// initialize pre-sorted data structures from input data
    void initData(const vector<float*>& ps);
    /// set up bounding cuts from dataset range
    BoxTreeNode* boundData(double xr = 0, BoxTreeNode* T = nullptr) const;

    /// Recursively divide supplied node to partition points; return top divided node, and counts by leaf
    BoxTreeNode* buildKD(map<const BoxTreeNode*,double>& leafcounts, BoxTreeNode* T = nullptr);

    vector<float*> psorted[20]; ///< input dataset, sorted along each axis (will be shuffled in building process)

protected:
    /// partition internal dataset range at split point on axis
    void partition(size_t N0, size_t N1, size_t Nc, int ax);

    /// Recursively divide supplied node to partition points; return top divided node, and counts by leaf
    BoxTreeNode* buildKD(size_t N0, size_t N1, map<const BoxTreeNode*,double>& leafcounts, BoxTreeNode* T = nullptr);
};

/////////////////////////
/////////////////////////

template<typename ptr_t>
BoxTreeNode::iterator_t<ptr_t>& BoxTreeNode::iterator_t<ptr_t>::operator++() {
    if(!N) return *this;
    if(!hiSide.size()) { N = nullptr; return *this; }
    N = N->parent;
    if(!N) return *this;
    if(!hiSide.back()) {
        N = N->cHi;
        hiSide.back() = true;
        descend_low();
    } else hiSide.pop_back();
    return *this;
}

template<typename ptr_t>
void BoxTreeNode::iterator_t<ptr_t>::descend_low() {
    while(N->cLo) {
        N = N->cLo;
        hiSide.push_back(false);
    }
}
