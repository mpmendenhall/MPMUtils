/// \file AxisEnum.hh enumeration for specifying/iterating over (x,y,z,...) axes
#ifndef AXISENUM_HH
#define AXISENUM_HH

#include <array>

/// axis directions
enum AxisDirection_t {
    X_DIRECTION = 0,
    Y_DIRECTION = 1,
    Z_DIRECTION = 2,
    T_DIRECTION = 3
};
/// iteration to next axis
inline AxisDirection_t& operator++(AxisDirection_t& d) { return d = AxisDirection_t(d+1); }
/// Iterable axes for 2 dimensions
constexpr std::array<AxisDirection_t, 2> Axes_2D{X_DIRECTION,Y_DIRECTION};
/// Iterable axes for 3 dimensions
constexpr std::array<AxisDirection_t, 3> Axes_3D{X_DIRECTION,Y_DIRECTION,Z_DIRECTION};

#endif
