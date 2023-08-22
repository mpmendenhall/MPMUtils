/// \file VisrGL.hh OpenGL visualization window driver
// -- Michael P. Mendenhall, 2019

#ifndef VISRGL_HH
#define VISRGL_HH

#include "Visr.hh"
#include <string>
using std::string;

#ifdef WITH_OPENGL
#include <deque>
#include <pthread.h>
#endif

/// Callback base during generic pause operation
class VGLCallback {
public:
    virtual ~VGLCallback() { }

    enum Callback_t {
        STARTMOUSE,
        MOVEMOUSE,
        KEYPRESS
    } reason;

    int x, y, a, b;
};

/// OpenGL window visualization driver
class GLVisDriver: public VisDriver {
public:
    /// start interactive drawing loop thread
    void doGlutLoop();
    /// stop interactive drawing loop thread
    void endGlutLoop();
    /// pause for user interaction
    void pause() override { pause(nullptr); }
    /// pause for user interaction, with optional callbacks function
    void pause(void (*f)(VGLCallback*), VGLCallback* args = nullptr);

    static const string _pause_info;    ///< default pause help
    string pause_info = _pause_info;    ///< info to display on pause

    //-///////////////////
    // viewing window info

    string windowTitle = "OpenGL Viewer Window";    ///< viewing window title
    float win_c[3] = {0,0,0};   ///< center of window view
    float ar = 1.0;             ///< (x range)/(y range) window aspect ratio
    float viewrange = 0;        ///< half-height (y) range
    float win_lo[3] = {0,0,0};  ///< Window lower range
    float win_hi[3] = {0,0,0};  ///< Window upper range
    int winwidth = 0;           ///< window pixel width
    int winheight = 0;          ///< window pixel height

#ifdef WITH_OPENGL
    static constexpr bool hasGL = true;

    /// Constructor
    GLVisDriver();

    /// Destructor
    ~GLVisDriver() { endGlutLoop(); }

    /// Display driver info
    void display() const override;

    //-/////////////////////////////////
    // actions called in GLUT event loop

    /// Flush queue and redraw display if queue unlocked
    void tryFlush();
    /// Redraw display
    void updateViewWindow();
    void redrawDisplay();
    void reshapeWindow(int width, int height);
    void keypress(unsigned char key, int x, int y);
    void specialKeypress(int key, int x, int y);
    void startMouseTracking(int button, int state, int x, int y);
    void mouseTrackingAction(int x, int y);
    /// reset rotations/scale to default
    void resetViewTransformation();

protected:

    /// add to backend commands execution queue
    void pushCommand(const VisCmd& c) override;

    /// start a group of related drawing commands
    void _startRecording(const vector<float>&) override;
    /// end a group of related drawing commands
    void _stopRecording(const vector<float>&) override;
    /// clear output --- automatic on startRecording(true)
    void _clearWindow(const vector<float>&) override;
    /// set color for subsequent draws
    void _setColor(const vector<float>&) override;
    /// draw series of lines between vertices
    void _lines(const vector<float>&) override;
    /// draw ball at location
    void _ball(const vector<float>&) override;
    /// OpenGL teapot
    void _teapot(const vector<float>&) override;

    /// get current transformation matrix
    void getMatrix();

    void (*pause_callback)(VGLCallback*) = nullptr; ///< callback during pause
    VGLCallback* pause_args = nullptr;              ///< callback arguments

    bool updated = true;            ///< flag to refresh updated drawing
    int clickx0 = 0, clicky0 = 0;   ///< click (start) location
    int modifier = 0;               ///< modifier key

    std::deque<VisCmd> commands;    ///< to-be-processed commands
    pthread_mutex_t commandLock;    ///< commands queue lock
    std::vector<unsigned int> displaySegs;  ///< OpenGL display segment identifiers
#else
    static constexpr bool hasGL = false;
#endif
};

#endif
