/// @file Visr.cc
// -- Michael P. Mendenhall, 2019

#include "Visr.hh"
#include "GeomCalcUtils.hh" // for ortho_frame

VisDriver::vec3 operator*(const VisDriver::vec3& v, double s) { return VisDriver::vec3{v[0]*s,v[1]*s,v[2]*s}; }

void appendv(std::vector<float>& v, VisDriver::vec3 a) {
    v.push_back(a[0]);
    v.push_back(a[1]);
    v.push_back(a[2]);
}

void VisDriver::teapot(double s) {
    VisCmd c(&VisDriver::_teapot, {float(s)});
    pushCommand(c);
}

void VisDriver::setColor(float r, float g, float b, float a) {
    pushCommand(VisCmd(&VisDriver::_setColor, {r,g,b,a}));
}

void VisDriver::clearWindow(float r, float g, float b, float a) {
    pushCommand(VisCmd(&VisDriver::_clearWindow, {r,g,b,a}));
}

void VisDriver::startRecording(bool newseg) {
    VisCmd c(&VisDriver::_startRecording);
    if(newseg) c.v.push_back(1); // mark as addition to previous segment
    pushCommand(c);
}

void VisDriver::stopRecording() {
    pushCommand(VisCmd(&VisDriver::_stopRecording));
}

void VisDriver::lines(const vector<vec3>& v, bool closed) {
    VisCmd c(&VisDriver::_lines);
    for(auto& p: v) appendv(c.v, p*scale);
    c.v.push_back(closed);
    pushCommand(c);
}

void VisDriver::circle(vec3 o, vec3 n, int i, double th0) {
    int j0 = 0;
    for(auto j: {1,2}) if(fabs(n[j]) > fabs(n[j0])) j0 = j;
    vec3 dz{0,0,0};
    dz[(j0+1)%3] = 1;

    auto r = makeunit(n);
    vec3 dx;
    vec3 dy;
    ortho_frame(dz, n, dx, dy);

    vector<vec3> v;
    for(int p=0; p<i; p++) {
        double th = th0 + p*2*M_PI/i;
        double c = r*cos(th);
        double s = r*sin(th);
        v.push_back(o);
        for(auto j: {0,1,2}) v.back()[j] += dx[j]*c + dy[j]*s;
    }
    lines(v, true);
}

void VisDriver::ball(vec3 p, double r, int nx, int ny) {
    VisCmd c(&VisDriver::_ball);
    appendv(c.v, p*scale);
    c.v.push_back(r*scale);
    c.v.push_back(nx);
    c.v.push_back(ny);
    pushCommand(c);
}
