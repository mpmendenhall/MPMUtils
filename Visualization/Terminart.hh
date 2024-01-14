/// @file Terminart.hh Color terminal "ASCII art" utilities
// -- Michael P. Mendenhall, 2020

#ifndef TERMINART_HH
#define TERMINART_HH

#include "SGRManip.hh"
#include "ColorSpec.hh"
#include <utility> // for std::pair
#include <limits>
#include <stdexcept>
#include <cmath> // std::isfinite

namespace Terminart {

    /// rendering specification for character "pixel"
    struct pixel_t {
        /// constructor
        pixel_t(char _c = 0): c(_c) { }

        /// set 24-bit color
        void set(const color::rgb32& crgb, bool fg = true);
        /// set enumerated 256-choices color
        void set(unsigned char col, bool fg = true);
        /// set 8-bit (256 color) approximant
        void set256(const color::rgb& crgb, bool fg = true);

        char c;     ///< character to display (0 for "blank" default)
        TermSGR s;  ///< display style
    };

    /// row,column location, from top left = 0,0
    struct rowcol_t: public std::pair<int,int> {
        using pair::pair;
        /// sum of row,col coordinates
        constexpr rowcol_t operator+(const rowcol_t& b) const { return {first+b.first, second+b.second}; }
        /// add-assign
        void operator+=(const rowcol_t& b) { *this = *this + b; }
        /// difference between row,col coordinates
        constexpr rowcol_t operator-(const rowcol_t& b) const { return {first-b.first, second-b.second}; }
        /// subtract-assign
        void operator-=(const rowcol_t& b) { *this = *this - b; }
        /// unary minus
        constexpr rowcol_t operator-() const { return {-first, -second}; }
        /// valid non-negative finite dimensions?
        constexpr bool isValidDim() const { return first >= 0 && second >= 0 && std::isfinite(first) && std::isfinite(second); }
        /// print to stdout
        void display() const { printf("(%i,%i)", first, second); }
    };

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

        /// print to stdout
        void display() const { first.display(); second.display(); }
    };

    /// a null interval rectangle
    static constexpr rectangle_t null_rectangle{{0,0}, {-1,-1}};
    /// integer infinity
    static constexpr int int_infty = std::numeric_limits<int>::infinity();
    /// infinite interval rectangle
    static constexpr rectangle_t infinite_rectangle{{-int_infty, -int_infty}, {int_infty, int_infty}};

    /// compositing rules for combining pixels
    class Compositor {
    public:
        /// Polymorphic destructor
        virtual ~Compositor() { }
        /// Return b layered over a at position x
        virtual pixel_t operator()(const pixel_t& a, const pixel_t& b, rowcol_t) const { return b.c? b : a; }

        static const Compositor Cdefault;   ///< default compositor
    };

    /// pixel buffer virtual base interface, with drawing primitives
    class VPixelBuffer {
    public:
        /// default constructor
        VPixelBuffer() { }
        /// valid dimension check constructor
        VPixelBuffer(rowcol_t dim) { if(!dim.isValidDim()) throw std::logic_error("Invalid pixel buffer dimensions"); }
        /// polymorphic destructor
        virtual ~VPixelBuffer() { }

        /// mutable element access
        virtual pixel_t& operator()(rowcol_t x) = 0;
        /// const element access
        virtual pixel_t operator()(rowcol_t x) const = 0;
        /// composite pixel into buffer
        void cput(rowcol_t x, const pixel_t& p, const Compositor& C) { (*this)(x) = C((*this)(x), p, x); }

        pixel_t p_xtra;     ///< extra pixel returned for out-of-bounds element access

        /// draw horizontal line
        void hline(rowcol_t x0, int dx, pixel_t p, const Compositor& C = Compositor::Cdefault);
        /// draw vertical line
        void vline(rowcol_t x0, int dy, pixel_t p, const Compositor& C = Compositor::Cdefault);
        /// framed rectangle
        void drawFrame(rectangle_t r, const pixel_t& c = {'+'}, const pixel_t& h = {'-'}, const pixel_t& v = {'|'},
                       const Compositor& C = Compositor::Cdefault);

    };

    /// fixed-size rectangular array of characters
    class pixelarray_t: public VPixelBuffer, protected vector<pixel_t> {
    public:
        /// constructor
        explicit pixelarray_t(rowcol_t _dim): VPixelBuffer(_dim), vector(_dim.first * _dim.second), dim(_dim) { }
        /// bounds checking
        bool inbounds(rowcol_t x) const { return 0 <= x.first && x.first < dim.first && 0 <= x.second && x.second < dim.second; }
        /// element access
        pixel_t& operator()(rowcol_t x) override { return inbounds(x)? (*this)[x.second + x.first*dim.second] : p_xtra; }
        /// element access
        pixel_t operator()(rowcol_t x) const override { return inbounds(x)? (*this)[x.second + x.first*dim.second] : p_xtra; }
        /// composite other array over this, starting at position x0 (in this array)
        void composite(rowcol_t x0, const pixelarray_t& o, const Compositor& C = Compositor::Cdefault);

        using vector::size;
        using vector::begin;
        using vector::end;

        const rowcol_t dim; ///< array dimensions, nRows x nCols

        /// render with terminal control codes
        string render(const string& newline = "\n", char cnull = ' ') const;
    };

    /// sparse collection of display pixels
    class pixelmap_t: public VPixelBuffer, public map<rowcol_t, pixel_t> {
    public:
        /// Constructor, with optional text block
        explicit pixelmap_t(const string& s = "");
        /// mutable element access
        pixel_t& operator()(rowcol_t x) override { return (*this)[x]; }
        /// const element access
        pixel_t operator()(rowcol_t x) const override { auto it = find(x); return it == end()? p_xtra : it->second; }
    };

    /// base class providing pixels in a viewport
    class TermViewport {
    public:
        /// polymorphic destructor
        ~TermViewport() { }

        /// composite this into array at position p0
        virtual void getView(rowcol_t p0, pixelarray_t& v, const Compositor& C = Compositor::Cdefault) const = 0;
        /// get bounding box
        virtual rectangle_t getBounds() const { return infinite_rectangle; }

        /// viewport with placement
        struct placement_t: public rowcol_t {
            using rowcol_t::rowcol_t;
            TermViewport* V = nullptr;
        };

        /// render bounding box to pixel array
        virtual pixelarray_t toArray() const;
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
        /// render bounding box to pixel array
        pixelarray_t toArray() const override { return *this; }
    };

    /// viewport over pixels map
    class MapViewport: public TermViewport, public pixelmap_t {
        /// inherit pixel array constructors
        using pixelmap_t::pixelmap_t;

        /// get bounding box
        rectangle_t getBounds() const override;
        /// fill viewport vector starting at p0 with given dimensions
        void getView(rowcol_t p0, pixelarray_t& v, const Compositor& C = Compositor::Cdefault) const override;
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


    // cursor control codes

    /// clear terminal and move cursor to bottom left
    static constexpr const char* clear_to_bl = "\033[2J";
    /// save current cursor position
    static constexpr const char* save_cpos = "\033[s";
    /// restore saved cursor position
    static constexpr const char* restore_cpos = "\033[u";

    /// cursor relative movement control string
    string cmove_control(rowcol_t x);
    /// cursor absolute position control string
    string cpos_control(rowcol_t x);
}

#endif
