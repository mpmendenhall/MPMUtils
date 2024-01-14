/// @file SGRManip.hh "Select Graphic Rendition" terminal text styles manipulation
// -- Michael P. Mendenhall, 2020

#ifndef SGRMANIP_HH
#define SGRMANIP_HH

#include <stdio.h>
#include <vector>
using std::vector;
#include <string>
using std::string;

/// Terminal color specification
class SGRColor {
public:
    /// Constructor, for foreground or background
    explicit SGRColor(bool _fg = true): fg(_fg) { }

    bool fg;    ///< specifies foreground color?

    /// color specification mode
    enum mode_t {
        COLOR_DEFAULT   = -1,   ///< user-defined default color
        COLOR_BLACK     = 0,    ///< 4-bit color, black
        COLOR_RED       = 1,
        COLOR_GREEN     = 2,
        COLOR_YELLOW    = 3,
        COLOR_BLUE      = 4,
        COLOR_MAGENTA   = 5,
        COLOR_CYAN      = 6,
        COLOR_WHITE     = 7,
        COLOR_8         = 8,    ///< 256-color palette
        COLOR_24        = 24    ///< 24-bit RrGgBb
    } mode = COLOR_DEFAULT;

    int color = 0;              ///< "bright" in mode <= 7; color identifier in COLOR_8/COLOR_32

    /// whether reset to default is needed
    bool needsReset(const SGRColor& C) const { return mode == COLOR_DEFAULT && C.mode != COLOR_DEFAULT; }

    /// SGR commands necessary to transform C to this state; returns whether reset issued
    bool diff(const SGRColor& C, vector<int>& v) const;
};

/// Terminal font specifications
class SGRFont {
public:
    /// Constructor
    SGRFont() { }

    /// font weight
    enum weight_t {
        WEIGHT_DIM  = 2,    ///< dim
        WEIGHT_MED  = 22,   ///< medium/default
        WEIGHT_BOLD = 1     ///< bold
    } weight = WEIGHT_MED;

    /// font family
    enum family_t {
        FAMILY_PLAIN = 23,  ///< plain/default
        FAMILY_ITALIC = 3,  ///< italic
        FAMILY_FRAKTUR = 20 ///< fraktur
    } family = FAMILY_PLAIN;

    /// whether underlined
    bool underline = false;

    /// blinky style
    enum blinky_t {
        BLINKY_HELLNO = 25, ///< not blinking
        BLINKY_BLINK = 5,   ///< slow blink
        CRAZY_BLINK = 6     ///< fast blink
    } blinky = BLINKY_HELLNO;


    /// whether concealed
    bool concealed = false;
    /// whether inverted
    bool inverted = false;
    /// whether strikethrough
    bool stricken = false;

    /// SGR commands necessary to transform F to this state
    void diff(const SGRFont& F, vector<int>& v) const;
};

/// Select Graphic Rendition state and manipulation
class TermSGR {
public:
    /// Constructor, to default state
    TermSGR(): FG(true), BG(false) { }

    SGRColor FG;    ///< foreground color
    SGRColor BG;    ///< background color
    SGRFont  Font;  ///< font

    /// SGR commands necessary to transform S to this state
    string diff(const TermSGR& S) const;
};

#endif
