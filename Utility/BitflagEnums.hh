/// @file BitflagEnums.hh Macro for setting up bit flag operations
// M.P. Mendenhall, 2022

#ifndef BITFLAGENUMS_HH
#define BITFLAGENUMS_HH

#define BITFLAGIZE(base_t, flags_t)                                                 \
enum flags_t: int { };                                                                   \
inline constexpr flags_t operator|(flags_t f, flags_t o) { return flags_t(int(f)|o); }   \
inline constexpr flags_t operator&(flags_t f, flags_t o) { return flags_t(int(f)&o); }   \
inline void operator|=(flags_t& f, flags_t o) { f = f|o; }                          \
inline void operator&=(flags_t& f, flags_t o) { f = f&o; }                          \
inline constexpr flags_t base_t##_flag(base_t b) { return flags_t(1 << b); }        \
inline constexpr flags_t operator|(flags_t f, base_t o)  { return f|base_t##_flag(o); } \
inline constexpr flags_t operator&(flags_t f, base_t o)  { return f&base_t##_flag(o); } \
inline void operator|=(flags_t& f, base_t o) { f = f|o; }                           \
inline void operator&=(flags_t& f, base_t o) { f = f&o; }

#endif
