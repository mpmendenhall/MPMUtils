/// \file Terminart.hh Color terminal "ASCII art" utilities
// -- Michael P. Mendenhall, 2020

#ifndef TERMINART_HH
#define TERMINART_HH

#include "SGRManip.hh"
#include "ColorSpec.hh"
#include <utility> // for std::pair
#include <map>
using std::map;

namespace Terminart {

    /// row,column location, from top left = 0,0
    typedef std::pair<int,int> rowcol_t;

    /// rendering specification for character "pixel"
    struct pixel_t {
        /// constructor
        pixel_t(char _c = 0): c(_c) { }

        /// set color
        void set(const color::rgb32& crgb, bool fg = true);

        char c;     ///< character to display (0 for "blank" default)
        TermSGR s;  ///< display style

    };

    /// sparse collection of display pixels
    class TermPic: public map<rowcol_t, pixel_t> {
    public:
        /// Constructor, with optional text block
        TermPic(const string& s = "");

        /// repeated char row [c0,c1]
        void drawRow(const pixel_t& p, int r, int c0, int c1) { if(c1 < c0) std::swap(c1,c0); while(c0 <= c1) (*this)[{r, c0++}] = p; }
        /// repeated char column [r0,r1]
        void drawCol(const pixel_t& p, int c, int r0, int r1) { if(r1 < r0) std::swap(r1,r0); while(r0 <= r1) (*this)[{r0++, c}] = p; }
        /// frame with corners at p0, p1
        void drawFrame(rowcol_t p0, rowcol_t p1, const pixel_t& c = {'+'}, const pixel_t& h = {'-'}, const pixel_t& v = {'|'});

        /// "blank space" defaults
        pixel_t p_default;

        /// print out
        void display() const;
    };
}

#endif
