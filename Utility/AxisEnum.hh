/// \file AxisEnum.hh enumeration for specifying/iterating over (x,y,z,...) axes
#ifndef AXISENUM_HH
#define AXISENUM_HH

#include <array>
using std::array;
#include "BitflagEnums.hh"

/// axis directions
enum AxisDirection_t {
    X_DIRECTION = 0,
    Y_DIRECTION = 1,
    Z_DIRECTION = 2,
    T_DIRECTION = 3
};
/// iteration to next axis
inline AxisDirection_t& operator++(AxisDirection_t& d) { return d = AxisDirection_t(d+1); }
// axis name characters
constexpr array<char, 4> Axis_Name_Chars{'x','y','z','t'};
/// Iterable axes for 2 dimensions
constexpr array<AxisDirection_t, 2> Axes_2D{X_DIRECTION,Y_DIRECTION};
/// Other axis for 2 dimensions
constexpr array<AxisDirection_t, 2> Axes_2D_b{Y_DIRECTION,X_DIRECTION};
/// Iterable axes for 3 dimensions
constexpr array<AxisDirection_t, 3> Axes_3D{X_DIRECTION,Y_DIRECTION,Z_DIRECTION};
/// First axis permutation, 3 dimensions
constexpr array<AxisDirection_t, 3> Axes_3D_b{Y_DIRECTION,Z_DIRECTION,X_DIRECTION};
/// Second axis permutation, 3 dimensions
constexpr array<AxisDirection_t, 3> Axes_3D_c{Z_DIRECTION,X_DIRECTION,Y_DIRECTION};

/// axis direction combination bitflags
BITFLAGIZE(AxisDirection_t, AxisSelection_t)

#endif
