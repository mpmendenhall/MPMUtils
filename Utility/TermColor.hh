/// \file TermColor.hh Defines for terminal text color control codes
// -- Michael P. Mendenhall, 2020

#ifndef TERMCOLOR_HH
#define TERMCOLOR_HH

/// "Control Sequence Inducer" ESC[
#define ANSI_CSI "\x1b["

/// "Select Graphic Rendition" CSI command
#define ANSI_CSI_SGR(...) ANSI_CSI __VA_ARGS__ "m"

// reset SGR
#define ANSI_SGR_RESET     "0"

/// bold
#define ANSI_SGR_BOLD      "1"
/// dim
#define ANSI_SGR_DIM       "2"
/// normal weight
#define ANSI_SGR_MEDIUM    "22"

/// italic
#define ANSI_SGR_ITALIC    "3"
/// fraktur
#define ANSI_SGR_FRAKTUR   "20"
/// non-italic/fraktur
#define ANSI_SGR_PLAIN     "23"


/// underline
#define ANSI_SGR_UNDERLINE "4"
/// no underline
#define ANSI_SGR_UNLINE    "24"

/// slow blink
#define ANSI_SGR_BLINK     "5"
/// fast blink
#define ANSI_SGR_FLICKER   "6"
/// non-blink
#define ANSI_SGR_UNBLINK   "25"

/// invert color
#define ANSI_SGR_INVERT    "7"
/// end invert
#define ANSI_SGR_UNVERT    "27"

/// strikethrough
#define ANSI_SGR_CONCEAL   "8"
/// non-strikethrough
#define ANSI_SGR_REVEAL    "28"

/// strikethrough
#define ANSI_SGR_STRIKE    "9"
/// non-strikethrough
#define ANSI_SGR_UNSTRIKE  "29"

// "10" -- "19" : font choices, 10 is default.

// foreground colors
#define ANSI_FG_BLACK   "30"
#define ANSI_FG_RED     "31"
#define ANSI_FG_GREEN   "32"
#define ANSI_FG_YELLOW  "33"
#define ANSI_FG_BLUE    "34"
#define ANSI_FG_MAGENTA "35"
#define ANSI_FG_CYAN    "36"
#define ANSI_FG_WHITE   "37"

// foreground bright colors
#define ANSI_FG_BBLACK   "90"
#define ANSI_FG_BRED     "91"
#define ANSI_FG_BGREEN   "92"
#define ANSI_FG_BYELLOW  "93"
#define ANSI_FG_BBLUE    "94"
#define ANSI_FG_BMAGENTA "95"
#define ANSI_FG_BCYAN    "96"
#define ANSI_FG_BWHITE   "97"

// background colors
#define ANSI_BG_BLACK   "40"
#define ANSI_BG_RED     "41"
#define ANSI_BG_GREEN   "42"
#define ANSI_BG_YELLOW  "43"
#define ANSI_BG_BLUE    "44"
#define ANSI_BG_MAGENTA "45"
#define ANSI_BG_CYAN    "46"
#define ANSI_BG_WHITE   "47"

// background bright colors
#define ANSI_BG_BBLACK   "100"
#define ANSI_BG_BRED     "101"
#define ANSI_BG_BGREEN   "102"
#define ANSI_BG_BYELLOW  "103"
#define ANSI_BG_BBLUE    "104"
#define ANSI_BG_BMAGENTA "105"
#define ANSI_BG_BCYAN    "106"
#define ANSI_BG_BWHITE   "107"

// shortcuts

#define TERMFG_RED     ANSI_CSI_SGR(ANSI_FG_RED)
#define TERMFG_GREEN   ANSI_CSI_SGR(ANSI_FG_GREEN)
#define TERMFG_YELLOW  ANSI_CSI_SGR(ANSI_FG_YELLOW)
#define TERMFG_BLUE    ANSI_CSI_SGR(ANSI_FG_BLUE)
#define TERMFG_MAGENTA ANSI_CSI_SGR(ANSI_FG_MAGENTA)
#define TERMFG_CYAN    ANSI_CSI_SGR(ANSI_FG_CYAN)

#define TERMSGR_RESET  ANSI_CSI_SGR(ANSI_SGR_RESET)

/// 24-bit RGB foreground
#define ANSI_SGR_FG_RGB(R, G, B) ANSI_CSI_SGR("38;2;" #R ";" #G ";" #B)
/// 24-bit RGB background
#define ANSI_SGR_BG_RGB(R, G, B) ANSI_CSI_SGR("48;2;" #R ";" #G ";" #B)

#endif
