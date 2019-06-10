/// \file Icosahedral.hh Icosahedral symmetry manipulations
// Michael P. Mendenhall, 2019


#ifndef ICOSAHEDRAL_HH
#define ICOSAHEDRAL_HH

/*

#include "Matrix.hh"
#include "PhiField.hh"
#include "ModularField.hh"
#include "ConjugacyDecomposition.hh"
#include "QuotientGroup.hh"
#include "DecisionTree.hh"
#include "GeomCalcUtils.hh"

/// unit value for PhiField --- needed for matrix inverse
template<>
inline PhiField unit<PhiField>() { return PhiField::one(); }

/// Information about full icosahedral symmetry point group
/// Schoenflies I_h = I x C_2 (= A_5 x Z_2), Coxeter [5,3]
namespace Icosahedral {
    using std::array;

    /// Symmetry group element
    typedef Matrix<3,3,PhiField> elem_t;
    /// Group operation (matrix multiplication)
    typedef MultiplyG<elem_t> groupop_t;
    /// Rotation axis type
    typedef Vec<3,PhiField> axis_t;
    /// Number of icosahedral symmetry elements
    constexpr size_t n_elements = 120;

    // matrix representation generators for full icosahedral symmetry
    extern const elem_t Ra; ///< one rotation generator
    extern const elem_t Rb; ///< another rotation generator
    extern const elem_t Rc; ///< inversion generator

    /// matrix generators span type
    typedef GeneratorsSemigroup<groupop_t> genspan_t;
    /// arbitrary enumeration of 120 rotation matrices representing icosahedral point group
    extern const genspan_t Rs;
    /// Cayley Table type for icosahedral group
    typedef CayleyTable<genspan_t> cayley_t;
    /// Precalculated Cayley Table (abstract representation of group structure)
    extern const cayley_t CT;
    /// Conjugacy classes decomposition type
    typedef ConjugacyDecomposition<cayley_t> conjugacy_t;
    /// Precalculated decomposition
    extern const conjugacy_t CD;

    /// Quotient groups (using Cayley Table)
    typedef QuotientGroup<cayley_t> quotient_t;

    /// identity element number
    constexpr size_t nID = 0;

    /// enumerated element
    struct indexel_t {
        /// constructor from index
        indexel_t(size_t ii = nID);

        size_t i;   ///< element index
        elem_t o;   ///< element representation
    };

    /// Mirror-reflection parity (false: flips; true: no flips) by enumerated generator
    extern const struct s_parity: public array<bool,n_elements> { s_parity(); } parity;

    /// Information on related operators defining a face/edge
    template<size_t O, size_t C>
    struct faceinfo_t {
        /// operator order (edges per face)
        static constexpr size_t order() { return O; }
        /// operator conjugacy multiplicity (number of faces)
        static constexpr size_t multiplicity() { return C; }

        axis_t    c{};        ///< central axis (fixed point of R[...])
        indexel_t R[O];       ///< ID and successive face rotations: stabilizer subgroup w.r.t. c
        indexel_t g[O];       ///< elements moving fundamental domain to each domain of this face, R[i]*g[0]
    };

    template<typename _face>
    struct faceset_t: public array<_face, _face::multiplicity()> {
        /// constructor with conjugacy class number
        faceset_t(size_t cnum);
        /// face number for element
        size_t facenum(size_t e) const { return elenum[e]/_face::order(); }
        /// group elements into faces
        array<size_t, n_elements> elenum{};
    };

    /// dodecahedral face info
    typedef faceinfo_t<5,12> f12_t;
    extern const faceset_t<f12_t> dodFaces;

    /// flip axis info
    typedef faceinfo_t<2,15> f15_t;
    extern const faceset_t<f15_t> flipAxes;

    /// icosahedral face info
    typedef faceinfo_t<3,20> f20_t;
    extern const faceset_t<f20_t> icoFaces;

    /// apply all 60 (120) group elements to vector, eliminating duplicates
    template<typename V>
    vector<V> points(const V& v, bool posparity = false) {
        vector<V> vv(Rs.getOrder()/(posparity? 2 : 1));
        auto it = vv.begin();
        size_t i = 0;
        for(auto& M: Rs) {
            if(!parity[i++] && posparity) continue;
            *(it++) = Matrix<3,3,array_contents_t<V>>(M)*v;
        }
        std::sort(vv.begin(), vv.end());
        vv.erase(std::unique(vv.begin(), vv.end()), vv.end());
        return vv;
    }

    /// cos theta for rotation
    inline PhiField cosTheta(const elem_t& M) { return (M.trace()-1)/2; }

    /// print description of icosahedral symmetry to stdout
    void describe();

    /// Point classification into 120 domains (corresponding to enumerated group elements), cut by 15 dividing hemispheres
    /// arbitrary choice of which domain is "fundamental"
    class Navigator: public DecisionTree {
    public:
        /// Constructor
        Navigator();

        /// Identify domain in which vector falls (operator index from fundamental domain to v's domain)
        template<class V>
        size_t domain(const V& v) const { return decide(v, axpart<V>); }

        /// Map item to fundamental domain; return operator number to map v back to its starting position
        template<class V>
        size_t map_d0(V& v) const {
            auto dmn = domain(v);
            v = Matrix<3,3,array_contents_t<V>>(Rs.element(CD.inverse_idx(dmn)))*v;
            return dmn;
        }

        // three corners of fundamental domain, named by size of orbit under G
        static const f12_t v12; ///< corner of fundamental domain centered on dodecahedral face
        static const f15_t v15; ///< corner of fundamental domain centered on flip edge
        static const f20_t v20; ///< corner of fundamental domain centered on icosahedral face

    protected:
        /// check direction of vector relative to flip axis
        template<class V>
        static bool axpart(const V& v, size_t t) { return dot(v, flipAxes[t].c) < 0; }
    };
    /// Pre-constructed Navigator
    extern const Navigator Nav;

    /// Icosahedral Voronoi nearest-direction finder, from list of points in fundamental domain
    template<typename V>
    class NNFinder: public vector<V> {
    public:
        /// inherit constructors
        using vector<V>::vector;

        /// find {domain, nearest point index} for u -> u mapped to fundamental domain
        template<typename U>
        std::pair<size_t, size_t> map_NN(U& u) {
            assert(this->size());
            auto dmn = Nav.map_d0(u);
            return {dmn, std::min_element(this->begin(), this->end(),
                    [&u](const V& x) { return direction_d2(u, x); })-this->begin()};
        }
    };

    /// (unnormalized) barycentric coordinate in a domain, w0*v12 + w1*v15 + w2*v20
    template<typename T>
    struct bcoord_t: public Vec<3,T> {
        size_t n = 0;   ///< operator index to map point from fundamental domain to proper location

        using Vec<3,T>::Vec;

        /// position in fundamental domain
        Vec<3,T> v0() const { return Vec<3,T>(Navigator::v12.c) * (*this)[0] + Vec<3,T>(Navigator::v15.c) * (*this)[1] + Vec<3,T>(Navigator::v20.c) * (*this)[2]; }
        /// position
        Vec<3,T> v() const { return Matrix<3,3,T>(Rs.element(n))*v0(); }
    };
}
*/

#endif

// TODO
// operators to move into each face
// operators to walk to adjacent faces

/*
 * Polyhedral groups:
 * tetrahedron isomorphic to A4
 * octohedral (+cube): orientation-preserving chiral isormorphic to S4 (24 elements), full has 48 elements
 * icosohedral (+dodecahedron): isomorphic to A5
 */


