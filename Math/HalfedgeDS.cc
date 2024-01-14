/// @file HalfedgeDS.cc
// -- Michael P. Mendenhall, LLNL 2019

#include "HalfedgeDS.hh"
#include <stdexcept>
#include <tuple> // for std::ignore

HalfedgeDS::HalfedgeDS(size_t n) {
    if(!n) return;

    auto& v0 = new_vertex();
    auto& e0 = new_fulledge(v0, v0);
    e0.next = &e0;
    e0.opposite->next = e0.opposite;
    new_face(e0);
    new_face(*e0.opposite);
    f_outer = e0.face;

    while(--n) split_edge(e0);
    validate();
}

HalfedgeDS::vertex_t& HalfedgeDS::new_vertex() {
    vs.emplace_front();
    return vs.front();
}

HalfedgeDS::edge_t& HalfedgeDS::new_tadpole(vertex_t& v) {
    es.emplace_front();
    edge_t& e = es.front();
    v.in.insert(&e);
    v.out.insert(&e);
    e.to = &v;
    e.next = e.opposite = &e;

    new_face(e);
    return e;
}

HalfedgeDS::edge_t& HalfedgeDS::new_halfedge(vertex_t& v0, vertex_t& v1) {
    es.emplace_front();
    edge_t& e = es.front();
    e.to = &v1;
    v0.out.insert(&e);
    v1.in.insert(&e);
    return e;
}

HalfedgeDS::edge_t& HalfedgeDS::new_fulledge(vertex_t& v0, vertex_t& v1) {
    edge_t& e = new_halfedge(v0, v1);
    new_halfedge(v1, v0).setOpposite(e);
    return e;
}

void HalfedgeDS::new_face(edge_t& e) {
    fs.emplace_front(e);
    e.face = &fs.front();
}

void HalfedgeDS::split_edge(edge_t& e) {
    vertex_t& v0 = *e.from();
    vertex_t& vm = new_vertex();
    vertex_t& v1 = *e.to;

    edge_t& eo = *e.opposite;
    e.to = eo.to = &vm;

    edge_t& ea = new_halfedge(vm,v1);
    ea.next = e.next;
    ea.setOpposite(eo);
    ea.face = e.face;
    e.next = &ea;

    edge_t& eb = new_halfedge(vm,v0);
    eb.next = eo.next;
    eb.setOpposite(e);
    eb.face = eo.face;
    eo.next = &eb;

    v0.in.erase(&eo);
    v1.in.erase(&e);
    vm.in.insert(&e);
    vm.in.insert(&eo);

    validate();
}

HalfedgeDS::edge_t& HalfedgeDS::split_face(edge_t& e1, edge_t& e2) {
    if(e1.face != e2.face) throw std::logic_error("Edges not on same face");

    edge_t& e = new_fulledge(*e2.to, *e1.from());
    new_face(e);

    edge_t& eo = *e.opposite;
    eo.face = e1.face;
    e1.face->edge = &eo;
    eo.next = e2.next;

    e2.next = &e;
    e.next = &e1;

    edge_t* ee = &e1;
    while(ee != &e2) {
        ee->face = e.face;
        ee = ee->next;
    }
    e2.face = e.face;

    ee = eo.next;
    while(ee->next != &e1) ee = ee->next;
    ee->next = &eo;

    validate();

    return e;
}

//void HalfedgeDS::delete_edge(eit_t e) {
//}

void HalfedgeDS::validate() {
    // vertex in/out lists consistency
    for(auto& v: vs) {
        if(v.in.size() != v.out.size()) throw std::logic_error("Mismatched in/out lists");
        for(auto e: v.in) if(e->to != &v) throw std::logic_error("'in' edge points to different vertex");
        for(auto e: v.out) if(e->from() != &v) throw std::logic_error("'out' edge comes from different vertex");
    }

    // edge pointing consistency
    for(auto& e: es) {
        if(e.opposite->opposite != &e) throw std::logic_error("Mismatched opposites");
        if(e.next->opposite->to != e.to) throw std::logic_error("Inconsistent edge pointing");
        if(!e.to->in.count(&e)) throw std::logic_error("Edge missing from vertex 'in' list");
        if(!e.from()->out.count(&e)) throw std::logic_error("Edge missing from vertex 'out' list");
    }

    // edge "next" cycle shares same face
    for(auto& f: fs) {
        auto e0 = f.edge;
        if(e0->face != &f) throw std::logic_error("Edge/face mismatch");
        auto e = e0->next;
        while(e != e0) {
            if(e->face != &f) {
                printf("Edge %p pointing to face %p (should be %p?)\n", (void*)e, (void*)e->face, (void*)&f);
                throw std::logic_error("Inconsistent faces assignment");
            }
            e = e->next;
        }
    }
}

template<typename T>
size_t len(const T& x) {
    size_t s = 0;
    for(auto& u: x) { std::ignore = u; ++s; }
    return s;
}

void HalfedgeDS::display(bool verbose) const {
    if(verbose) printf("\n--------------------------------------\n");
    printf("Half-edge data structure with %zu vertices, %zu half-edges, and %zu faces\n", len(vs), len(es), len(fs));
    if(!verbose) return;
    for(auto& v: vs) v.display();

    set<const edge_t*> visited;

    for(auto& f: fs) {
        if(&f == f_outer) printf("Outer ");
        f.display();
        auto e = f.edge;
        while(!visited.count(e)) {
            e->display();
            visited.insert(e);
            e = e->next;
        }
    }

    for(auto& e: es) {
        if(visited.count(&e)) continue;
        printf("*** FLOATING EDGE ****\n");
        e.display();
    }
}

void HalfedgeDS::vertex_t::display() const {
    printf("*   Vertex %p: in { ", (void*)this);
    for(auto e: in) printf("%p ", (void*)e);
    printf("} out { ");
    for(auto e: out) printf("%p ", (void*)e);
    printf("}\n");
}

void HalfedgeDS::edge_t::display() const {
    printf(" -  Edge %p: from %p to %p, opposite %p next %p (face %p)\n",
           (void*)this, (void*)from(), (void*)to, (void*)opposite, (void*)next, (void*)face);
}

void HalfedgeDS::face_t::display() const {
    printf("  @ Face %p with edge %p\n",
           (void*)this, (void*)edge);
}

void HalfedgeDS::split_all_edges() {
    // clear edge flags
    for(auto& e: es) e.flags = 0;

    // add new vertex splitting every full edge (flags to mark split opposite)
    for(auto& e: es) {
        if(e.flags) continue;
        e.opposite->flags = 1;
        split_edge(e);
    }

    validate();
}

void HalfedgeDS::split_corners(face_t& f, edge_t* e) {
    if(!e) e = f.edge;
    else if(e->face != &f) throw std::logic_error("Edge does not belong to face");

    auto v0 = e->to;
    e = e->next;

    do {
        auto e1 = e->next;
        auto en = e1->next;
        split_face(*e, *e1);
        e = en;
    } while(e->from() != v0);
}
