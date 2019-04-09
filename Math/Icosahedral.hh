/// \file Icosahedral.hh Icosahedral symmetry manipulations
// Michael P. Mendenhall, 2019

#ifndef ICOSAHEDRAL_HH
#define ICOSAHEDRAL_HH

#include "Matrix.hh"
#include "PhiField.hh"
#include "ModularField.hh"
#include "FiniteGroup.hh"
#include "DecisionTree.hh"
#include "GeomCalcUtils.hh"

/// Information about icosahedral symmetry point group
namespace Icosahedral {
    using std::array;

    /// Symmetry group element
    typedef Matrix<3,3,PhiField> elem_t;
    /// Group operation (matrix multiplication)
    typedef MultiplySG<elem_t> groupop_t;
    /// Rotation axis type
    typedef Vec<3,PhiField> axis_t;

    /// triangular rotations enumeration
    typedef ModularField<3>  n3_t;
    /// pentagon rotations enumeration
    typedef ModularField<5>  n5_t;
    /// dodecahedral faces / icosahedral vertices enumeration
    typedef ModularField<12> n12_t;
    /// dodecahedral flips enumeration
    typedef ModularField<15> n15_t;
    /// icosahedral faces = dodecahedral vertices enumeration
    typedef ModularField<20> n20_t;
    /// icosahedral, dodecahedral edges enumeration
    typedef ModularField<30> n30_t;

    // generators for full icosahedral symmetry
    extern const elem_t Ra; ///< one rotation generator
    extern const elem_t Rb; ///< another rotation generator
    extern const elem_t Rc; ///< inversion generator

    /// generators span type
    typedef GeneratorsSemigroup<groupop_t> genspan_t;
    /// All 60 rotation matrices in icosahedral point group
    extern const genspan_t Rs;
    /// Cayley Table type for icosahedral rotations
    typedef CayleyTable<genspan_t> cayley_t;
    /// Precalculated Cayley Table
    extern const cayley_t CT;
    /// Conjugacy classes decomposition type
    typedef ConjugacyDecomposition<cayley_t> conjugacy_t;
    /// Precalculated decomposition
    extern const conjugacy_t CD;

    /// identity element number
    constexpr size_t nID = 0;

    /// indexed element
    struct indexel_t {
        /// null constructor
        indexel_t(): i{}, o{} { }
        /// constructor from index
        indexel_t(size_t ii);

        size_t i;   ///< element index
        elem_t o;   ///< element representation
    };

    /// Mirror-reflection parity (false: flips; true: no flips) by enumerated generator
    extern const array<bool,120> parity;

    /// Information on related operators defining a face/edge
    template<size_t O, size_t C>
    struct faceinfo_t {
        /// operator order
        static constexpr size_t order = O;
        /// operator conjugacy multiplicity
        static constexpr size_t multiplicity = C;

        axis_t    c;        ///< central axis (fixed point of R[...])
        indexel_t R[O];     ///< ID and successive face rotations: stabilizer subgroup w.r.t. c
        //axis_t    vs[O];    ///< vertices, clockwise loop
    };

    /// dodecahedral face info
    typedef faceinfo_t<5,12> f12_t;
    extern const array<f12_t,12> dodFaces;

    /// flip axis info
    typedef faceinfo_t<2,15> f15_t;
    extern const array<f15_t,15> flipAxes;

    /// icosahedral face info
    typedef faceinfo_t<3,20> f20_t;
    extern const array<f20_t,20> icoFaces;

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

    /// Point identification
    class Navigator: public DecisionTree {
    public:
        /// Constructor
        Navigator();

        /// Identify domain in which vector falls
        template<class V>
        size_t domain(const V& v) const { return decide(v, axpart<V>); }
        /// Map item to fundamental domain; return operator number to map v back to its starting position
        template<class V>
        size_t map_d0(V& v) const {
            auto dmn = domain(v);
            v = Matrix<3,3,array_contents_t<V>>(Rs.element(CD.inverse_idx(dmn)))*v;
            return dmn;
        }

        // three corners of fundamental domain
        static const f12_t v12; ///< dodecahedral face corner of fundamental domain
        static const f15_t v15; ///< flip edge corner of fundamental domain
        static const f20_t v20; ///< icosahedral face corner of fundamental domain

    protected:
        /// check direction of vector relative to flip axis
        template<class V>
        static bool axpart(const V& v, size_t t) { return dot(v, flipAxes[t].c) < 0; }
    };
    /// Pre-constructed Navigator
    extern const Navigator Nav;

    /// (unnormalized) barycentric coordinate in a domain, w0*v12 + w1*v15 + w2*v20
    template<typename T>
    struct bcoord_t: public Vec<3,T> {
        size_t n = 0;   ///< operator index to map point from fundamental domain to proper location

        /// position in fundamental domain
        Vec<3,T> v0() const { return Vec<3,T>(Navigator::v12.c) * (*this)[0] + Vec<3,T>(Navigator::v15.c) * (*this)[1] + Vec<3,T>(Navigator::v20.c) * (*this)[2]; }
        /// position
        Vec<3,T> v() const { return Matrix<3,3,T>(Rs.element(n))*v0(); }
    };
}

/*
 * Polyhedral groups:
 * tetrahedron isomorphic to A4
 * octohedral (+cube): orientation-preserving chiral isormorphic to S4 (24 elements), full has 48 elements
 * icosohedral (+dodecahedron): isomorphic to A5
 */

#endif
