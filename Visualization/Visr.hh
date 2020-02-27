/// \file Visr.hh Base class for minimalist 3D visualization interface
// Michael P. Mendenhall, LLNL 2019

#ifndef VISR_HH
#define VISR_HH

#include <vector>
using std::vector;
#include <array>
#include <stdexcept>

/// Generic minimalist 3D visualization driver interface
class VisDriver {
public:
    /// Destructor
    virtual ~VisDriver() { }

    /// coordinate type
    typedef std::array<double,3> vec3;

    /// start recording a series of draw commands; newseg=true to erase all previous series
    void startRecording(bool newseg = false);
    /// stop recording a series of draw commands
    void stopRecording();
    /// pause for user interaction
    virtual void pause() { }
    /// Display driver info
    virtual void display() const { printf("Unspecified visualization driver\n"); }

    /// clear window to blank screen --- automatic on startRecording(true)
    void clearWindow(float r = 1, float g = 1, float b = 1, float a = 1);
    /// set color for subsequent draws
    void setColor(float r, float g, float b, float a = 1);
    /// lines or polygon
    void lines(const vector<vec3>& v, bool closed = false);
    /// draw specified line
    void line(vec3 s, vec3 e) { lines({s,e}); }
    /// draw ball at location
    void ball(vec3 p, double r, int nx = 8, int ny = 8);
    /// draw circle (polygon) with center o, normal/radius n; i line segments
    void circle(vec3 o, vec3 n, int i = 36, double th0 = 0);
    /// draw teapot (OpenGL only!)
    void teapot(double s = 1.0);

    /// Global drawing re-scale
    float scale = 1.0;

    /// Coordinates projection matrix (row-major order)
    float mProj[4][4];

protected:

    /// Queued drawing command for a driver
    struct VisCmd {
        /// Constructor
        VisCmd(void (VisDriver::*f)(const vector<float>&), const vector<float>& _v = {}): fcn(f), v(_v) { }

        void (VisDriver::*fcn)(const vector<float>&);   ///< function to call
        vector<float> v;                                ///< function arguments
    };

    //-/////////////////////////////////////////////////
    // implement in derived classes to do the real work!

    /// add/process (possibly deferred) command
    virtual void pushCommand(const VisCmd& c) { (this->*c.fcn)(c.v); }

    /// start a group of related drawing commands
    virtual void _startRecording(const vector<float>& v) { if(v.size()) _clearWindow(v); }
    /// end a group of related drawing commands
    virtual void _stopRecording(const vector<float>&) { }
    /// clear output --- automatic on startRecording(true)
    virtual void _clearWindow(const vector<float>&) { }
    /// set color for subsequent draws
    virtual void _setColor(const vector<float>&) { }
    /// draw series of lines between vertices
    virtual void _lines(const vector<float>&) { }
    /// draw ball at location
    virtual void _ball(const vector<float>&) { }
    /// OpenGL teapot
    virtual void _teapot(const vector<float>&) { }
};

#endif
