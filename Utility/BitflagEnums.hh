/// @file BitflagEnums.hh Macro for setting up bit flag operations
// M.P. Mendenhall, 2022

#ifndef BITFLAGENUMS_HH
#define BITFLAGENUMS_HH

#define BITFLAGIZE(base_t, flags_t)                                                 \
enum flags_t { };                                                                   \
inline constexpr flags_t operator|(flags_t s, flags_t o) { return flags_t(int(s)|o); }   \
inline constexpr flags_t operator&(flags_t s, flags_t o) { return flags_t(int(s)&o); }   \
inline void operator|=(flags_t& s, flags_t o) { s = s|o; }                          \
inline void operator&=(flags_t& s, flags_t o) { s = s&o; }                          \
inline constexpr flags_t base_t##_flag(base_t b) { return flags_t(1 << b); }        \
inline constexpr flags_t operator|(flags_t s, base_t o)  { return s|base_t##_flag(o); } \
inline constexpr flags_t operator&(flags_t s, base_t o)  { return s&base_t##_flag(o); } \
inline void operator|=(flags_t& s, base_t o) { s = s|o; }                           \
inline void operator&=(flags_t& s, base_t o) { s = s&o; }

#endif
