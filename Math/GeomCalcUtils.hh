/// \file GeomCalcUtils.hh Geometry calculation utilities
// Michael P. Mendenhall, 2019

#ifndef GEOMCALCUTILS_HH
#define GEOMCALCUTILS_HH

#include <cmath>
#include <type_traits>
#include <numeric> // for std::accumulate

template<typename T>
using array_contents_t = typename std::remove_reference<decltype(std::declval<T&>()[0])>::type;

/// generic vector magnitude^2
template<typename V>
array_contents_t<V> vmag2(const V& v) {
    typedef array_contents_t<V> T;
    T m2{};
    return std::accumulate(v.begin(), v.end(), m2, [](T a, T b) { return a + b*b; });
}

/// Vector dot product
template<typename T>
inline array_contents_t<T> dot(const T& a,  const T& b) {
    array_contents_t<T> d{};
    auto it = b.begin();
    for(auto v: a) d += v * *(it++);
    return d;
}

/// Vector difference a-b
template<typename T>
T vdiff(const T& a, const T& b) {
    T d = a;
    auto itb = b.begin();
    for(auto& x: d) x -= *(itb++);
    return d;
}

/// vector cross product
template<typename T>
inline void cross(const T& a, const T& b, T& c) {
    static_assert(std::tuple_size<T>::value == 3, "Cross-product only defined for 3-vector");
    c[0] = a[2]*b[1] - a[1]*b[2];
    c[1] = a[2]*b[0] - a[0]*b[2];
    c[2] = a[0]*b[1] - a[1]*b[0];
}
/// vector triple product
template<typename T>
inline array_contents_t<T> triple_prod(const T& a,  const T& b, const T& c) {
    static_assert(std::tuple_size<T>::value == 3, "Triple-product only defined for 3-vector");
    return a[0]*b[1]*c[2] + a[2]*b[0]*c[1] + a[1]*b[2]*c[0] - a[2]*b[1]*c[0] - a[1]*b[0]*c[2] - a[0]*b[2]*c[1];
}

/// 3-vector magnitude^2
template<typename T>
inline array_contents_t<T> mag2(const T& v) { return dot(v,v); }

/// vector magnitude
template<typename T>
inline array_contents_t<T> mag(const T& v) { return sqrt(mag2(v)); }
/// normalize to unit vector; return original length
template<typename T>
inline array_contents_t<T> makeunit(T& v) {
    auto d = mag(v);
    for(auto& x: v) x /= d;
    return d;
}

/// height of triangle
/**
 * @param b0 one point on triangle base
 * @param b1 other point on triangle base
 * @param h third triangle vertex
 * @return distance squared of h from line through b0, b1
 */
template<typename T>
inline array_contents_t<T> triangle_height2(const T& b0,  const T& b1, const T& h) {
    const auto d = vdiff(b1,b0);
    const auto v = vdiff(h,b0);
    auto d2 = dot(d,d);
    auto v2 = dot(v,v);
    auto x =  dot(d,v);
    return v2 - x*x/d2;
}

/// area of triangle
/**
 * @param b0 one point on triangle base
 * @param b1 other point on triangle base
 * @param h third triangle vertex
 * @return area^2 of the triangle
 */
template<typename T>
inline array_contents_t<T> triangle_area2(const T& b0,  const T& b1, const T& h) {
    const auto d = vdiff(b1,b0);
    const auto v = vdiff(h,b0);
    auto d2 = dot(d,d);
    auto v2 = dot(v,v);
    auto x =  dot(d,v);
    return 0.25*(v2*d2-x*x);
}

/// cosine of angle abc
/**
 * @param a starting point
 * @param b mid point
 * @param c end point
 * @return norm(b-a) dot norm(c-b)
 */
template<typename T>
inline array_contents_t<T> cos_abc(const T& a,  const T& b, const T& c) {
    const auto v0 = vdiff(b,a);
    const auto v1 = vdiff(c,b);
    return dot(v0,v1)/sqrt(dot(v0,v0)*dot(v1,v1));
}

/// decompose coordinate into projection along and distance from line
/**
 * @param c any point on the line
 * @param vn normal vector in line direction
 * @param x point position
 * @param z distance of x along line from c
 * @param r2 distance squared of x from closest point on line
 */
template<typename T>
inline void line_coords(const T& c,  const T& vn, const T& x, array_contents_t<T>& z, array_contents_t<T>& r2) {
    const auto d = vdiff(x,c);
    z = dot(d,vn);
    r2 = fabs(dot(d,d) - z*z);
}

/// decompose coordinate into projection along and distance from line segment
/**
 * @param c any point on the line
 * @param vn normal vector in line direction
 * @param x point position
 * @param z0 start of line segment, c + z0*vn
 * @param z1 end of line segment, c + z1*vn
 * @param z distance of x along line from c
 * @param r2 distance squared of x from closest point on line
 */
template<typename T>
inline void lineseg_coords(const T& c,  const T& vn, const T& x,
                           array_contents_t<T> z0, array_contents_t<T> z1, array_contents_t<T>& z, array_contents_t<T>& r2) {
    line_coords(c,vn,x,z,r2);
    if(z1 < z0) std::swap(z0,z1);
    if(z < z0) r2 += (z-z0)*(z-z0);
    if(z > z1) r2 += (z1-z)*(z1-z);
}

/// find closest points of approach between two lines
/**
 * @param p1 point on first line
 * @param d1 direction of first line
 * @param p2 point on second line
 * @param d2 direction of second line
 * @param[out] c1 closest approach is at p1 + c1*d1
 * @param[out] c2 closest approach is at p2 + c2*d2
 */
template<typename T>
void closest_approach_points(const T& p1, const T& d1,
                             const T& p2, const T& d2,
                             array_contents_t<T>& c1, array_contents_t<T>& c2) {
    auto d0  = vdiff(p2,p1);
    auto a01 = dot(d0,d1);
    auto a02 = dot(d0,d2);
    auto a11 = dot(d1,d1); // =1 for normalized directions
    auto a12 = dot(d1,d2);
    auto a12a12 = a12*a12;
    auto a22 = dot(d2,d2); // =1 for normalized directions

    auto dd = a11*a22 - a12a12;
    if(dd < 1e-6*a12a12) { // parallel lines special case
        c1 = c2 = 0;
        return;
    }

    c1 = (a22*a01 - a12*a02)/dd;
    c2 = (a12*a01 - a11*a02)/dd;
}

/// find closest points of approach between two lines, assuming directions are normalized
/**
 * @param p1 point on first line
 * @param d1 unit vector direction of first line
 * @param p2 point on second line
 * @param d2 unit vector direction of second line
 * @param[out] c1 closest approach is at p1 + c1*d1
 * @param[out] c2 closest approach is at p2 + c2*d2
 */
template<typename T>
void closest_approach_points_normalized(
    const T& p1, const T& d1,
    const T& p2, const T& d2,
    array_contents_t<T>& c1, array_contents_t<T>& c2) {

    auto d0 = vdiff(p2,p1[0]);
    auto a01 = dot(d0,d1);
    auto a02 = dot(d0,d2);
    auto a12 = dot(d1,d2);
    auto a12a12 = a12*a12;

    auto dd = 1 - a12a12;
    if(dd < 1e-6*a12a12) { // parallel lines special case
        c1 = c2 = 0;
        return;
    }

    c1 = (a01 - a12*a02)/dd;
    c2 = (a12*a01 - a02)/dd;
}

/// calculate closest approach distance squared after closest_approach_points call
/**
 * @param p1 point on first line
 * @param d1 direction of first line
 * @param p2 point on second line
 * @param d2 direction of second line
 * @param c1 first point at p1 + c1*d1
 * @param c2 second point at at p2 + c2*d2
 * @return distance^2 between first and second point
 */
template<typename T>
T line_points_distance2(const T& p1, const T& d1,
                        const T& p2, const T& d2,
                        array_contents_t<T> c1, array_contents_t<T> c2) {
    array_contents_t<T> s2{};
    size_t i = 0;
    for(auto x: p1) {
        T l = (x+c1*d1[i]) - (p2[i]+c2*d2[i]);
        s2 += l*l;
        ++i;
    }
    return s2;
}

/// Calculate tangential and radial unit vectors frame relative to specified unit direction and z axis
/**
 * @param va input unit vector
 * @param vt unit tangential vector (va x z)/|va x z|
 * @param vr unit radial vector va x vt
 */
template<typename T>
void local_polar_frame(const T& va, T& vt, T& vr) {
    static_assert(std::tuple_size<T>::value == 3, "Only defined for 3-vector");

    auto d = sqrt(va[0]*va[0] + va[1]*va[1]);
    vt[2] = 0;
    if(d > 1e-6) {
        vt[0] = -va[1]/d;
        vt[1] = va[0]/d;

        vr[0] = -vt[1]*va[2];
        vr[1] = vt[0]*va[2];
        vr[2] = vt[1]*va[0] - vt[0]*va[1];
    } else { // special axis-aligned case
        vt[0] = -1;
        vt[1] = 0;

        vr[0] = 0;
        vr[1] = -va[2];
        vr[2] = va[1];
    }
}

/// Orthogonal frame v0,v1,v2 given specified v0 and "up" hint vu
/**
 * @param vu input unit vector defining "up" direction
 * @param v0 input unit vector forming va axis
 * @param v1 unit vector (v0 x vu)/|v0 x vu|
 * @param v2 unit vector v0 x v1
 */
template<typename T>
void ortho_frame(const T& vu, const T& v0, T& v1, T& v2) {
    cross(v0,vu,v1);
    makeunit(v1);
    cross(v0,v1,v2);
}

#endif
