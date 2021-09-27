/// \file SGRManip.cc

#include "SGRManip.hh"

bool SGRColor::diff(const SGRColor& C, vector<int>& v) const {
    if(needsReset(C)) {
        v.push_back(0);
        diff(SGRColor(), v);
        return true;
    }
    if(mode == C.mode && color == C.color) return false;
    if(mode <= COLOR_WHITE) v.push_back(mode + (fg? 30 : 40) + (color? 60 : 0));
    else if(mode == COLOR_8) {
        v.push_back(fg? 38 : 48);
        v.push_back(5);
        v.push_back(color);
    } else if(mode == COLOR_24) {
        v.push_back(fg? 38 : 48);
        v.push_back(2);
        v.push_back((color & 0xFF0000) >> 16);
        v.push_back((color & 0xFF00) >> 8);
        v.push_back( color & 0xFF );
    }
    return false;
}

void SGRFont::diff(const SGRFont& F, vector<int>& v) const {
    if(weight != F.weight) v.push_back(weight);
    if(family != F.family) v.push_back(family);
    if(underline != F.underline) v.push_back(underline? 4 : 24);
    if(blinky != F.blinky) v.push_back(blinky);
    if(inverted != F.inverted)   v.push_back(inverted?  7 : 27);
    if(concealed != F.concealed) v.push_back(concealed? 8 : 28);
    if(stricken != F.stricken)   v.push_back(stricken?  9 : 29);
}

string TermSGR::diff(const TermSGR& S) const {
    vector<int> v;

    bool isReset = FG.diff(S.FG, v);
    isReset |= BG.diff(isReset? SGRColor(false) : S.BG, v);
    Font.diff(isReset? SGRFont() : S.Font, v);

    if(!v.size()) return "";

    char c[32];
    string s = "\x1b[";
    for(auto i: v) { snprintf(c, 32, "%i;", i); s += c; }
    s.back() = 'm';
    return s;
}
