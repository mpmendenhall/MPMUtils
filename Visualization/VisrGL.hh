/// \file VisrGL.hh OpenGL visualization window driver
// Michael P. Mendenhall, LLNL 2019

#ifndef VISRGL_HH
#define VISRGL_HH

#include "Visr.hh"
#include <string>
using std::string;

#ifdef WITH_OPENGL
#include <GL/freeglut.h>
#include <deque>
#include <pthread.h>
#endif

/// Callback base during generic pause operation
class VGLCallback {
public:
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
    /// initialize visualization window
    void initWindow(const std::string& title = "OpenGL Viewer Window");
    /// pause for user interaction
    void pause() override { pause(nullptr); }
    /// pause for user interaction, with optional callbacks function
    void pause(void (*f)(void*, VGLCallback*), void* args = nullptr);

    static const string _pause_info;    ///< default pause help
    string pause_info = _pause_info;    ///< info to display on pause

    //-///////////////////
    // viewing window info

    float win_c[3];     ///< center of window view
    float ar = 1.0;     ///< (x range)/(y range) window aspect ratio
    float viewrange;    ///< half-height (y) range
    float win_lo[3];    ///< Window lower range
    float win_hi[3];    ///< Window upper range
    int winwidth, winheight;    ///< pixel dimensions

#ifdef WITH_OPENGL
    static constexpr bool hasGL = true;

    /// Destructor
    ~GLVisDriver() { endGlutLoop(); }

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

    /// reset rotations/scale to default
    void resetViewTransformation();
    /// get current transformation matrix
    void getMatrix();

    void (*pause_callback)(void*, VGLCallback*) = nullptr;  ///< callback during pause
    void* pause_args = nullptr;                             ///< callback arguments

    bool updated = true;        ///< flag to refresh updated drawing
    int clickx0, clicky0;       ///< click (start) location
    int modifier;               ///< modifier key

    std::deque<VisCmd> commands;    ///< to-be-processed commands
    pthread_mutex_t commandLock;    ///< commands queue lock
    std::vector<GLuint> displaySegs;
#else
    static constexpr bool hasGL = false;
#endif
};

#endif
