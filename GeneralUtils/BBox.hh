#ifndef BBOX_HH
#define BBOX_HH

/// Templatized D-dimensional bounding box
template<size_t D, typename T>
class BBox {
public:
    /// Constructor
    BBox() { }
    
    /// expand to include point
    void expand(const T* x) {
        for(size_t i=0; i<D; i++) {
            if(x[i] < lo[i]) lo[i] = x[i];
            if(x[i] > hi[i]) hi[i] = x[i];
        }
    }
    
    /// check if point in half-open [lo, hi) interior
    bool inside(const T* x) {
        for(size_t i=0; i<D; i++)
            if(!(lo[i] <= x[i] && x[i] < hi[i])) return false;
        return true;
    }
    
    /// width along axis
    T dl(size_t i) const { return hi[i] - lo[i]; }
    /// local coordinate along axis, 0->lo and 1->hi
    T pos(T x, size_t i) const { return lo[i] + x*dl(i); }
    
    T lo[D];    /// lower bounds
    T hi[D];    /// upper bounds
};

template<size_t D>
BBox<D,double> empty_double_bbox() {
    BBox<D,double> b;
    for(size_t i=0; i<D; i++) {
        b.lo[i] = DBL_MAX;
        b.hi[i] = -DBL_MAX;
    }
    return b;
}

#endif
