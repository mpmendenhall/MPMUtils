/// @file iffy_constexpr.hh disable constexpr where not supported in older standards

#ifndef _constexpr
#if __cplusplus >= 201400L
#define _constexpr constexpr
#else
#define _constexpr
#endif
#endif
