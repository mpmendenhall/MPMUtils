/// \file Terminart.cc

#include "Terminart.hh"

using namespace Terminart;

void rectangle_t::include(rowcol_t p) {
    if(isNull()) {
        first = second = p;
    } else {
        first.first = std::min(first.first, p.first);
        first.second = std::min(first.second, p.second);
        second.first = std::max(second.first, p.first);
        second.second = std::max(second.second, p.second);
    }
}

//----------------------------------
//----------------------------------

void pixel_t::set(const color::rgb32& crgb, bool fg) {
    auto& C = fg? s.FG : s.BG;
    C.mode = SGRColor::COLOR_24;
    C.color = crgb.as_rgb_i24();
}

void pixel_t::set(unsigned char col, bool fg) {
    auto& C = fg? s.FG : s.BG;
    C.mode = SGRColor::COLOR_8;
    C.color = col;
}

void pixel_t::set256(const color::rgb& crgb, bool fg) {
    set(16 + 36*int(5.99*crgb.r) + 6*int(5.99*crgb.g) + int(5.99*crgb.b), fg);
}

//----------------------------------
//----------------------------------

const Compositor Compositor::Cdefault;

void pixelarray_t::composite(rowcol_t x0, const pixelarray_t& o, const Compositor& C) {
    int rmax = std::min(dim.first, o.dim.first - x0.first);
    int cmax = std::min(dim.second, o.dim.second - x0.second);
    rowcol_t x;
    for(x.first = x0.first; x.first < rmax; ++x.first) {
        for(x.second = x0.second; x.second < cmax; ++x.second) {
            (*this)(x) = C((*this)(x), o(x-x0), x);
        }
    }
}

string pixelarray_t::render(const string& newline, char cnull) const {
    string s;

    TermSGR tprev;
    auto it = begin();
    for(int r = 0; r < dim.first; ++r) {
        for(int c = 0; c < dim.second; ++c) {
            s += it->s.diff(tprev);
            s += it->c? it->c : cnull;
            tprev = (it++)->s;
        }
        s += TermSGR().diff(tprev) + newline;
        tprev = TermSGR();
    }

    return s;
}

//----------------------------------
//----------------------------------

TermPic::TermPic(const string& s) {
    int row = 0;
    int col = 0;
    for(auto c: s) {
        if(c == '\n') { ++row; col = 0; continue; }
        (*this)[{row, ++col}] = pixel_t(c);
    }
}

void TermPic::drawFrame(rowcol_t p0, rowcol_t p1, const pixel_t& c, const pixel_t& h, const pixel_t& v) {
    drawRow(h, p0.first, p0.second+1, p1.second-1);
    drawRow(h, p1.first, p0.second+1, p1.second-1);

    drawCol(v, p0.second, p0.first+1, p1.first-1);
    drawCol(v, p1.second, p0.first+1, p1.first-1);

    (*this)[p0] = (*this)[p1] = c;
    std::swap(p0.first, p1.first);
    (*this)[p0] = (*this)[p1] = c;
}

void TermPic::getView(rowcol_t p0, pixelarray_t& v, const Compositor& C) const {

    auto p1 = p0 + v.dim;

    auto it = lower_bound(p0);
    while(it != end()) {
        if(it->first >= p1) break;
        if(it->first.second >= p1.second) {
            ++p0.first;
            it = lower_bound(p0);
            continue;
        }
        v(it->first - p0) = C(v(it->first - p0), it->second, it->first - p0);
        ++it;
    }
}

rectangle_t TermPic::getBounds() const {
    auto b = null_rectangle;
    for(auto& kv: *this) b.include(kv.first);
    return b;
}

void TermPic::display(pixel_t p_default) const {
    int row = 0;
    int col = 0;

    string s;
    TermSGR tprev;
    TermSGR t0;

    for(auto& kv: *this) {
        while(row < kv.first.first) {
            s += t0.diff(tprev);
            tprev = t0;
            s += "\n";
            ++row;
            col = 0;
        }

        if(col) {
            while(col++ < kv.first.second) {
                s += p_default.s.diff(tprev);
                tprev = p_default.s;
                s += p_default.c;
            }
        } else {
            while(col++ < kv.first.second) {
                s += t0.diff(tprev);
                tprev = t0;
                s += ' ';
            }
        }

        s += kv.second.s.diff(tprev);
        tprev = kv.second.s;
        s += (kv.second.c? kv.second.c : p_default.c);
    }

    s += TermSGR().diff(tprev);
    printf("%s\n", s.c_str());
}

//----------------------

rectangle_t MultiViewport::getBounds() const {
    auto b = null_rectangle;
    for(auto& P: *this) {
        if(!P.V) continue;
        auto bb = P.V->getBounds();
        b.include(bb.first + P);
        b.include(bb.second + P);
    }
    return b;
}
