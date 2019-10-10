/// \file VisrGL.hh OpenGL visualization window driver
// Michael P. Mendenhall, LLNL 2019

#ifndef VISRGL_HH
#define VISRGL_HH

#include "Visr.hh"
#include "VisrSVG.hh"

#ifdef WITH_OPENGL
#include <GL/freeglut.h>

#include <deque>

/// OpenGL window visualization driver
class GLVisDriver: public VisDriver {
public:
    /// Constructor
    GLVisDriver();
    /// Destructor
    ~GLVisDriver();

    /// initialize visualization window
    void initWindow(const std::string& title = "OpenGL Viewer Window");

    /// start a group of related drawing commands
    void startRecording(const vector<float>&) override;
    /// end a group of related drawing commands
    void stopRecording(const vector<float>&) override;

    /// reset rotations/scale to default
    void resetViewTransformation() override;
    /// clear output --- automatic on startRecording(true)
    void clearWindow(const vector<float>&) override;
    /// set color for subsequent draws
    void setColor(const vector<float>&) override;
    /// draw series of lines between vertices
    void lines(const vector<float>&) override;
    /// draw ball at location
    void ball(const vector<float>&) override;

    /// OpenGL teapot
    void teapot(const vector<float>&) override;
    /// pause for user interaction
    void pause() override;

    std::vector<GLuint> displaySegs;

    /// add to backend commands execution queue
    void pushCommand(const VisCmd& c) override;

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

    /// Window range
    float win_x0, win_y0, win_x1, win_y1, win_z0 = 0, win_z1 = 10;

protected:
    /// get current transformation matrix
    void getMatrix();

    /// flag to maintain display pause
    bool pause_display = false;
    int clickx0, clicky0;
    int modifier;
    int winwidth, winheight;
    bool updated = true;

    /// to-be-processed commands
    std::deque<VisCmd> commands;
    /// commands queue lock
    pthread_mutex_t commandLock;
};

/// enter main drawing loop
void doGlutLoop();
/// exit from main drawing loop
void endGlutLoop();

#else

/// Null visualization driver without OpenGL
class GLVisDriver: public VisDriver { }

/// enter (null) main drawing loop
void doGlutLoop() { }
/// exit (null) from main drawing loop
void endGlutLoop() { }

#endif

#endif
