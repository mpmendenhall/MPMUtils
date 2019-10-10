/// \file Visr.hh Simple OpenGL visualization window
// Michael P. Mendenhall, 2018

#ifndef VISR_HH
#define VISR_HH

#include <vector>
using std::vector;
#include <array>
#include <stdexcept>

struct VisCmd;

/// Generic visualization driver interface
class VisDriver {
public:
    /// Destructor
    virtual ~VisDriver() { }

    /// add/process (possibly deferred) command
    virtual void pushCommand(const VisCmd& c);

    /// start a group of related drawing commands
    virtual void startRecording(const vector<float>& v) { if(v.size()) clearWindow(v); }
    /// end a group of related drawing commands
    virtual void stopRecording(const vector<float>&) { }
    /// pause for user interaction
    virtual void pause() { }

    /// reset rotations to defaults
    virtual void resetViewTransformation() { }
    /// clear output --- automatic on startRecording(true)
    virtual void clearWindow(const vector<float>&) { }
    /// set color for subsequent draws
    virtual void setColor(const vector<float>&) { }
    /// draw series of lines between vertices
    virtual void lines(const vector<float>&) { }
    /// draw ball at location
    virtual void ball(const vector<float>&) { }
    /// OpenGL teapot
    virtual void teapot(const vector<float>&) { }

    float ar = 1.0;         ///< (x range)/(y range) window aspect ratio
    float viewrange;        ///< half-height (y) range
    float xrot,yrot,zrot;
    float xtrans, ytrans;   ///< x/y center of view

    /// Coordinates projection matrix (row-major order)
    float mProj[4][4];
};

/// Queued drawing command for a driver
struct VisCmd {
    /// Constructor
    VisCmd(void (VisDriver::*f)(const vector<float>&), const vector<float>& _v = {}): fcn(f), v(_v) { }

    /// function to call
    void (VisDriver::*fcn)(const vector<float>&);
    /// function arguments
    vector<float> v;
};

/// global visualization namespace
namespace vsr {
    /// Active driver
    extern VisDriver* theDriver;

    /// coordinate type
    typedef std::array<double,3> vec3;

    /// Global drawing scale
    extern float scale;

    /// reset view to default
    void resetViewTransformation();
    /// update view window after changes
    void updateViewWindow();
    /// start recording a series of draw commands; newseg=true to erase all previous series
    void startRecording(bool newseg = false);
    /// stop recording a series of draw commands
    void stopRecording();
    /// wait for [ENTER] to be pushed in vis window
    void pause();
    /// screendump to .tga-format file
    void screendump(const char* fname = "screendump.tga");

    /// clear window to blank screen --- automatic on startRecording(true)
    void clearWindow(float r = 1, float g = 1, float b = 1, float a = 1);
    /// set color for subsequent draws
    void setColor(float r, float g, float b, float a = 1);
    /// set solid/wireframe mode
    inline void setWireframe(bool) { }
    /// draw specified line
    void line(vec3 s, vec3 e);
    /// lines or polygon
    void lines(const vector<vec3>& v, bool closed = false);
    /// draw specified plane, centered at o, +/- x and y vectors
    void plane(vec3 o, vec3 dx, vec3 dy);
    /// draw quadrangle (wireframe or filled)
    void quad(float* xyz);
    /// draw dot at location
    void dot(vec3 p);
    /// draw ball at location
    void ball(vec3 p, double r, int nx = 8, int ny = 8);
    /// draw circle (polygon) with center o, normal/radius n; i line segments
    void circle(vec3 o, vec3 n, int i = 36, double th0 = 0);
    /// draw teapot
    void teapot(double s = 1.0);
}

#endif
