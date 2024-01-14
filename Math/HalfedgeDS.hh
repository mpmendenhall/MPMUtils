/// @file HalfedgeDS.hh Halfedge data structure
// -- Michael P. Mendenhall, LLNL 2019

#ifndef HALFEDGEDS_HH
#define HALFEDGEDS_HH

#include <forward_list>
using std::forward_list;
#include <set>
using std::set;
#include <stdlib.h> // for size_t

/// Halfedge data structure
class HalfedgeDS {
public:

    struct edge_t;

    /// Vertex definition
    struct vertex_t {
        set<edge_t*> in;    ///< in edges
        set<edge_t*> out;   ///< out edges

        /// print debugging info to stdout
        void display() const;
    };

    struct face_t;

    /// Edge definition
    struct edge_t {
        vertex_t* to = nullptr;     ///< incident vertex
        /// origin vertex
        vertex_t* from() const { return opposite->to; }
        edge_t* next = nullptr;     ///< next edge
        edge_t* opposite = nullptr; ///< opposite edge
        face_t* face = nullptr;     ///< associated face
        int flags = 0;              ///< algorithm flags

        /// print debugging info to stdout
        void display() const;
        /// set edge as opposite
        void setOpposite(edge_t& e) { opposite = &e; e.opposite = this; }
    };

    /// Face definition: points to one (arbitrary) edge of face
    struct face_t {
        /// constructor from reference
        explicit face_t(edge_t& e): edge(&e) { }
        edge_t* edge;   ///< one face edge

        /// print debugging info to stdout
        void display() const;
    };

    /// Constructor, for one n-gon separating two faces (or none for n=0)
    explicit HalfedgeDS(size_t n = 1);

    /// add new unconnected vertex
    vertex_t& new_vertex();

    /// add "tadpole" self-conjugate new edge and new face to vertex
    ///   ----\   '
    ///  v  f  e
    ///   <---/
    edge_t& new_tadpole(vertex_t& v);

    /// split edge e, 2 new half-edges (a,b) are created
    ///
    ///   -e->  -n->             -e-> -a->  -n->
    /// v0    v1        ---->  v0    v    v1
    ///   <-o-                   <-b- <-o-
    void split_edge(edge_t& e);

    /// split every edge
    void split_all_edges();

    /// split face f, returning new edge e
    /// from b = e2->to to a = e1->from()
    /// adjoining new face f' (f edge set to e->opposite)
    ///
    ///  ---> a -e1-> ... -e2-> b --->
    ///       \\       f'      //
    ///        \\--<-- e --<--//
    ///         \-->-- o -->--/
    ///                f
    ///
    edge_t& split_face(edge_t& e1, edge_t& e2);

    /// split triangular corners off even-edged face
    void split_corners(face_t& f, edge_t* e = nullptr);

    /// validate structure assumptions; throw std::logic_error if failed
    void validate();
    /// print debugging info to stdout
    void display(bool verbose = false) const;

    forward_list<vertex_t> vs;  ///< the vertices
    forward_list<edge_t> es;    ///< the edges
    forward_list<face_t> fs;    ///< the faces
    face_t* f_outer = nullptr;  ///< ``outside perimeter'' face

protected:
    /// new half-edge without opposite between vertices
    edge_t& new_halfedge(vertex_t& v0, vertex_t& v1);
    /// new edge from v0 to v1 (with opposite); prev/next/face unassigned
    edge_t& new_fulledge(vertex_t& v0, vertex_t& v1);
    /// new face associated with edge
    void new_face(edge_t& e);
};

#endif
