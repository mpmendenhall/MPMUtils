/// \file Terminart.hh Color terminal "ASCII art" utilities
// -- Michael P. Mendenhall, 2020

#ifndef TERMINART_HH
#define TERMINART_HH

#include "SGRManip.hh"
#include "ColorSpec.hh"
#include <utility> // for std::pair
#include <map>
using std::map;
#include <vector>
using std::vector;
#include <limits>

namespace Terminart {

    /// row,column location, from top left = 0,0
    typedef std::pair<int,int> rowcol_t;
    /// sum of row,col coordinates
    inline rowcol_t operator+(const rowcol_t& a, const rowcol_t& b) { return {a.first+b.first, a.second+b.second}; }
    /// difference between row,col coordinates
    inline rowcol_t operator-(const rowcol_t& a, const rowcol_t& b) { return {a.first-b.first, a.second-b.second}; }

    /// rectangular pixel range
    struct rectangle_t: public std::pair<rowcol_t, rowcol_t> {
        /// inherit constructors
        using pair::pair;

        /// check for null interval
        bool isNull() const { return !(first.first <= second.first && first.second <= second.second); }
        /// get dimensions
        rowcol_t dim() const { return second - first; }

        /// enlarge as needed to include point
        void include(rowcol_t p);
        /// enlarge as needed to include rectangle
        void include(rectangle_t r) { if(!r.isNull()) { include(r.first); include(r.second); } }
    };

    /// a null interval rectangle
    static constexpr rectangle_t null_rectangle{{0,0}, {-1,-1}};
    /// integer infinity
    static constexpr int int_infty = std::numeric_limits<int>::infinity();
    /// infinite interval rectangle
    static constexpr rectangle_t infinite_rectangle{{-int_infty, -int_infty}, {int_infty, int_infty}};

    /// rendering specification for character "pixel"
    struct pixel_t {
        /// constructor
        pixel_t(char _c = 0): c(_c) { }

        /// set 24-bit color
        void set(const color::rgb32& crgb, bool fg = true);
        /// set enumerated 256-bit color
        void set(unsigned char col, bool fg = true);
        /// set 8-bit (256 color) approximant
        void set256(const color::rgb& crgb, bool fg = true);

        char c;     ///< character to display (0 for "blank" default)
        TermSGR s;  ///< display style
    };

    /// compositing rules for combining pixels
    class Compositor {
    public:
        /// Polymorphic destructor
        virtual ~Compositor() { }
        /// Return b layered over a at position x
        virtual pixel_t operator()(const pixel_t& a, const pixel_t& b, rowcol_t) const { return b.c? b : a; }

        static const Compositor Cdefault;   ///< default compositor
    };

    /// fixed-size rectangular array of characters
    class pixelarray_t: protected vector<pixel_t> {
    public:
        /// constructor
        pixelarray_t(rowcol_t _dim): vector(dim.first*dim.second), dim(_dim) { }
        /// element access, without bounds checking
        pixel_t& operator()(rowcol_t x) { return (*this)[x.second + x.first*dim.second]; }
        /// element access, without bounds checking
        pixel_t operator()(rowcol_t x) const { return (*this)[x.second + x.first*dim.second]; }
        /// composite other array over this, starting at position x0 (in this array)
        void composite(rowcol_t x0, const pixelarray_t& o, const Compositor& C = Compositor::Cdefault);

        using vector::size;
        using vector::begin;
        using vector::end;

        const rowcol_t dim; ///< array dimensions, nRows x nCols

        /// render with terminal control codes
        string render(const string& newline = "\n", char cnull = ' ') const;
    };

    /// base class providing pixels in a viewport
    class TermViewport {
    public:
        /// polymorphic destructor
        ~TermViewport() { }
        /// fill viewport vector starting at p0 with given dimensions
        virtual void getView(rowcol_t p0, pixelarray_t& v, const Compositor& C = Compositor::Cdefault) const = 0;
        /// get bounding box
        virtual rectangle_t getBounds() const { return infinite_rectangle; }

        /// viewport with placement
        struct placement_t: public rowcol_t {
            using rowcol_t::rowcol_t;
            TermViewport* V = nullptr;
        };
    };

    /// viewport over pixels array
    class ArrayViewport: public TermViewport, public pixelarray_t {
        /// inherit pixel array constructors
        using pixelarray_t::pixelarray_t;

        rowcol_t x0;    ///< starting position coordinate

        /// fill viewport vector starting at p0 with given dimensions
        void getView(rowcol_t p0, pixelarray_t& v, const Compositor& C = Compositor::Cdefault) const override {
            v.composite(x0 - p0, *this, C);
        }
        /// get bounding box
        rectangle_t getBounds() const override { return {x0, x0 + dim}; }
    };

    /// sparse collection of display pixels
    class TermPic: public map<rowcol_t, pixel_t>, public TermViewport {
    public:
        /// Constructor, with optional text block
        TermPic(const string& s = "");

        /// repeated char row [c0,c1]
        void drawRow(const pixel_t& p, int r, int c0, int c1) { if(c1 < c0) std::swap(c1,c0); while(c0 <= c1) (*this)[{r, c0++}] = p; }
        /// repeated char column [r0,r1]
        void drawCol(const pixel_t& p, int c, int r0, int r1) { if(r1 < r0) std::swap(r1,r0); while(r0 <= r1) (*this)[{r0++, c}] = p; }
        /// frame with corners at p0, p1
        void drawFrame(rowcol_t p0, rowcol_t p1, const pixel_t& c = {'+'}, const pixel_t& h = {'-'}, const pixel_t& v = {'|'});

        /// get bounding box
        rectangle_t getBounds() const override;
        /// fill viewport vector starting at p0 with given dimensions
        void getView(rowcol_t p0, pixelarray_t& v, const Compositor& C = Compositor::Cdefault) const override;

        /// print out
        void display(pixel_t p_default = {' '}) const;
    };

    /// placement of multiple sub-views
    class MultiViewport: public TermViewport, protected vector<TermViewport::placement_t> {
    public:
        /// fill viewport vector starting at p0 with given dimensions
        void getView(rowcol_t p0, pixelarray_t& v, const Compositor& C = Compositor::Cdefault) const override {
            for(auto& P: *this) if(P.V) P.V->getView(p0 - P, v, C);
        }
        /// get bounding box
        rectangle_t getBounds() const override;
    };
}

#endif
