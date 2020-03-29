/// \file Terminart.cc

#include "Terminart.hh"

using namespace Terminart;

void pixel_t::set(const color::rgb32& crgb, bool fg) {
    auto& C = fg? s.FG : s.BG;
    C.mode = SGRColor::COLOR_24;
    C.color = crgb.as_rgb_i24();
}

TermPic::TermPic(const string& s) {
    p_default.c = ' ';

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

void TermPic::display() const {
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
