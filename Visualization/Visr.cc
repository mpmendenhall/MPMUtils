/// \file Visr.cc
// Michael P. Mendenhall, LLNL 2019

#include "Visr.hh"
#include "GeomCalcUtils.hh"

void VisDriver::pushCommand(const VisCmd& c) {
    (this->*c.fcn)(c.v);
}

namespace vsr {

    vec3 operator*(const vec3& v, double s) { return vec3{v[0]*s,v[1]*s,v[2]*s}; }
    VisDriver* theDriver = nullptr;

    float scale = 1.0;
    bool wireframe = true;

    void pause() { if(theDriver) theDriver->pause(); }

    void addCmd(const VisCmd& c) {
        if(theDriver) theDriver->pushCommand(c);
    }

    void appendv(std::vector<float>& v, vec3 a) {
        v.push_back(a[0]);
        v.push_back(a[1]);
        v.push_back(a[2]);
    }

    void teapot(double s) {
        VisCmd c(&VisDriver::teapot);
        c.v.push_back(s);
        if(!wireframe) c.v.push_back(1);
        addCmd(c);
    }

    void setColor(float r, float g, float b, float a) {
        addCmd(VisCmd(&VisDriver::setColor, {r,g,b,a}));
    }

    void clearWindow(float r, float g, float b, float a) {
        addCmd(VisCmd(&VisDriver::clearWindow, {r,g,b,a}));
    }

    void startRecording(bool newseg) {
        VisCmd c(&VisDriver::startRecording);
        if(newseg) c.v.push_back(1); // mark as addition to previous segment
        addCmd(c);
    }

    void stopRecording() {
        addCmd(VisCmd(&VisDriver::stopRecording));
    }

    void line(vec3 s, vec3 e) {
        lines({s,e});
    }

    void lines(const vector<vec3>& v, bool closed) {
        VisCmd c(&VisDriver::lines);
        for(auto& p: v) appendv(c.v, p*scale);
        c.v.push_back(closed);
        addCmd(c);
    }

    void circle(vec3 o, vec3 n, int i, double th0) {
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

    void ball(vec3 p, double r, int nx, int ny) {
        VisCmd c(&VisDriver::ball);
        appendv(c.v, p*scale);
        c.v.push_back(r*scale);
        c.v.push_back(nx);
        c.v.push_back(ny);
        addCmd(c);
    }
}
