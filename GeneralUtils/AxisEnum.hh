/// \file AxisEnum.hh \brief enumeration for specifying/iterating over (x,y,z,...) axes
#ifndef AXISENUM_HH
#define AXISENUM_HH

/// axis directions
enum AxisDirection {
    X_DIRECTION = 0,
    Y_DIRECTION = 1,
    Z_DIRECTION = 2,
    T_DIRECTION = 3
};
/// iteration to next axis
inline AxisDirection& operator++(AxisDirection& d) { return d = AxisDirection(d+1); }

#endif
