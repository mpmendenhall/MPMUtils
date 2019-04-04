/// \file GeomCalcUtils.hh Geometry calculation utilities
// Michael P. Mendenhall, 2019

#ifndef GEOMCALCUTILS_HH
#define GEOMCALCUTILS_HH

#include <cmath>
#include <type_traits>
#include <numeric> // for std::accumulate
#include <cmath>   // for sqrt

/// Get type for contents of fixed array, std::array, std::vector
template<typename T>
using array_contents_t = typename std::remove_reference<decltype(std::declval<T&>()[0])>::type;

/// Vector dot product; for mixed vector types, convert to contents of first
template<typename T, typename U>
inline array_contents_t<T> dot(const T& a,  const U& b) {
    array_contents_t<T> d{};
    auto it = b.begin();
    for(auto v: a) d += v * array_contents_t<T>(*(it++));
    return d;
}

/// arithmetic types mag^2
template<typename T, typename std::enable_if<std::is_arithmetic<T>::value>::type* = nullptr>
inline T mag2(const T& x) { return x*x; }

/// vector magnitude^2
template<typename V, typename std::enable_if<!std::is_arithmetic<V>::value>::type* = nullptr>
inline array_contents_t<V> mag2(const V& v) {
    typedef array_contents_t<V> T;
    return std::accumulate(v.begin(), v.end(), T{}, [](const T& a, const T& b) { return a + b*b; });
}

/// vector magnitude
template<typename T>
inline array_contents_t<T> mag(const T& v) { return sqrt(mag2(v)); }

/// Vector sum a+b
template<typename T>
T vsum(const T& a, const T& b) {
    T d = a;
    auto itb = b.begin();
    for(auto& x: d) x += *(itb++);
    return d;
}

/// Vector sum a + k*b
template<typename T, typename U>
T vsum(const T& a, const U& k, const T& b) {
    T d = a;
    auto itb = b.begin();
    for(auto& x: d) x += (*(itb++))*k;
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

///////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////

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
    return (v2*d2-x*x)/4;
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
    auto a11 = dot(d1,d1); // =1 for normalized directions
    auto a12 = dot(d1,d2);
    auto a12a12 = a12*a12;
    auto a22 = dot(d2,d2); // =1 for normalized directions

    if(a12a12 == a11*a22) { // parallel lines special case
        c1 = a01/a11;
        c2 = {};
        return;
    }

    auto a02 = dot(d0,d2);
    auto dd = a11*a22 - a12a12;
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

    auto d0  = vdiff(p2,p1);
    auto a01 = dot(d0,d1);
    auto a12 = dot(d1,d2);

    // parallel lines special case
    if(a12 == 1) {
        c1 = a01;
        c2 = {};
        return;
    }

    auto a02 = dot(d0,d2);
    auto a12a12 = a12*a12;
    auto dd = 1 - a12a12;

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
inline array_contents_t<T> line_points_distance2(
    const T& p1, const T& d1,
    const T& p2, const T& d2,
    const array_contents_t<T>& c1,
    const array_contents_t<T>& c2) { return mag2(vdiff(vsum(p1,c1,d1), vsum(p2,c2,d2))); }

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

/// Return c minimizing |U - c V|^2
template<typename T>
inline array_contents_t<T> closest_approach(const T& u, const T& v) { return dot(u,v)/mag2(v); }

/// Solve |U - c V|^2 = k^2, in form c = a +- sqrt(b^2); returns a, k2 -> b^2
template<typename T>
array_contents_t<T> circle_ixn(const T& u, const T& v, array_contents_t<T>& k2) {
    auto uv = dot(u,v);
    auto vv = mag2(v);
    auto c = uv/vv;
    k2 = c*c + (k2 - dot(u,u))/vv;
    return c;
}

#endif
