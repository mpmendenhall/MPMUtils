/// \file VisrGL.cc
// -- Michael P. Mendenhall, 2019

#include "VisrGL.hh"

const string GLVisDriver::_pause_info =
"Visualizer controls:\n"
"  * [enter] continue\n"
"  * Click-and-drag: rotate\n"
"  * [ctrl]-click-and-drag horizontally: zoom\n"
"  * [shift]-click-and-drag: shift view center\n"
"  * [esc]   reset view\n";

#ifdef WITH_OPENGL

#include <unistd.h> // for usleep

#ifdef __APPLE__

#define GL_SILENCE_DEPRECATION
#include <GLUT/glut.h>

#else

#include <GL/glut.h>

#endif

/// singleton registered OpenGL window
GLVisDriver* GLDr = nullptr;
/// whether glut_init is complete
bool glutIsInitialized = false;
/// indicator to kill visualization thread
bool kill_flag = false;
/// visualization main loop thread
pthread_t vthread;
/// glut version number
int glutversion = -1;
/// OpenGL version name
string glversion = "[UNINITIALIZED]";

GLVisDriver::GLVisDriver() {
    pthread_mutexattr_t displayLockAttr;
    pthread_mutexattr_init(&displayLockAttr);
    pthread_mutexattr_settype(&displayLockAttr, PTHREAD_MUTEX_RECURSIVE);
    pthread_mutex_init(&commandLock,&displayLockAttr);
}

void GLVisDriver::display() const {
#ifdef FREEGLUT
    printf("Visualizer using OpenGL '%s', freeGLUT %i\n", glversion.c_str(), glutversion);
#else
    printf("Visualizer using OpenGL '%s', GLUT %i\n", glversion.c_str(), glutversion);
#endif
}

void GLVisDriver::endGlutLoop() {
    if(GLDr != this) return;

    kill_flag = true;
    pthread_join(vthread, nullptr);
    GLDr = nullptr;
}

void GLVisDriver::resetViewTransformation() {
    viewrange = 1.0;
    win_c[0] = win_c[1] = 0;
    win_c[2] = 5;
    glLineWidth(1.5/viewrange);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glTranslatef(0, 0, 1.0*viewrange);
    updated = true;
    updateViewWindow();
}

void GLVisDriver::_clearWindow(const vector<float>& v) {
    if(v.size() == 4) glClearColor(v[0], v[1], v[2], v[3]);
    else glClearColor(1, 1, 1, 1);
    glClearDepth(100.0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void _end_pause(void*, VGLCallback*) { }

void GLVisDriver::pause(void (*f)(void*, VGLCallback*), void* args) {
    pause_callback = f;
    pause_args = args;
    if(pause_info.size()) printf("%s", pause_info.c_str());
    while(pause_callback != &_end_pause) usleep(50000);
}

void GLVisDriver::_setColor(const vector<float>& v) {
    glColor4f(v[0],v[1],v[2],v[3]);
}

void GLVisDriver::_lines(const vector<float>& v) {
    glBegin(v.back()? GL_LINE_LOOP : GL_LINE_STRIP);
    for(size_t i = 0; i+3 < v.size(); i += 3) glVertex3f(v[i],v[i+1],v[i+2]);
    glEnd();
}

void GLVisDriver::_ball(const vector<float>& v) {
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glTranslatef(v[0],v[1],v[2]);
    //if(wireframe) glutWireSphere(v[3], v[4], v[5]);
    //else
    glutSolidSphere(v[3], v[4], v[5]);
    glPopMatrix();
}

void GLVisDriver::_teapot(const vector<float>& v) {
    if(v.size() == 2) glutSolidTeapot(v[0]);
    else glutWireTeapot(v[0]);
}

void GLVisDriver::_startRecording(const vector<float>& v) {
    glFlush();
    glFinish();

    while(v.size() && displaySegs.size()) {
        if(glIsList(displaySegs.back()))
            glDeleteLists(displaySegs.back(), 1);   // delete this one old display list
        displaySegs.pop_back();
    }

    if(!displaySegs.size()) {
        displaySegs.push_back(glGenLists(1));       // one new display list name
        glNewList(displaySegs.back(), GL_COMPILE);  // compile but do not immediately execute
        _clearWindow(v);                            // always start clear
    }
}

void GLVisDriver::_stopRecording(const vector<float>&) {
    glEndList();
    glutPostRedisplay();
    glFlush();
    glFinish();
}

void GLVisDriver::pushCommand(const VisCmd& c) {
    pthread_mutex_lock(&commandLock);
    commands.push_back(c);
    pthread_mutex_unlock(&commandLock);
}

void GLVisDriver::tryFlush() {
    // cancel redraw if commands being updated
    if(pthread_mutex_trylock(&commandLock)) return;

    // process any new commands
    updated |= commands.size();
    while(commands.size()) {
        auto f = commands.front().fcn;
        if(f) (this->*f)(commands.front().v);
        commands.pop_front();
    }
    pthread_mutex_unlock(&commandLock);

    if(updated) redrawDisplay();
    updated = false;
}

struct screendump_request {
    int h0 = 0;
    int w0 = 0;
    bool doDump = false;
    int ndumps = 0;
    char fname[1024];
} theSDR;

void GLVisDriver::redrawDisplay() {
    if(!displaySegs.size()) return;

    glCallLists(displaySegs.size(), GL_UNSIGNED_INT, displaySegs.data());
    glutSwapBuffers();
    glFlush();
    glFinish();

    if(!theSDR.doDump) return;

    printf("Saving %i x %i screendump to '%s'\n", winwidth, winheight, theSDR.fname);
    auto fout = fopen(theSDR.fname,"wb");

    std::vector<char> pbuff(3*winwidth*winheight);
    glGetError();
    glReadBuffer(GL_FRONT);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glPixelStorei(GL_PACK_ALIGNMENT, 1);
    glReadPixels(0, 0, winwidth, winheight, GL_BGR, GL_UNSIGNED_BYTE, pbuff.data());

    short fhead[] = { 0, 2, 0, 0, 0, 0, short(winwidth), short(winheight), 24 }; // .tga file header
    fwrite(fhead, sizeof(fhead), 1, fout);
    fwrite(pbuff.data(), pbuff.size(), 1, fout);
    fclose(fout);

    theSDR.doDump = false;
    auto w0 = theSDR.w0;
    auto h0 = theSDR.h0;
    theSDR.w0 = theSDR.h0 = 0;
    glutReshapeWindow(w0, h0);
}

void GLVisDriver::reshapeWindow(int width, int height) {
    glViewport(0,0,width,height);
    winwidth = width; winheight = height;
    ar = float(width)/float(height);
    updateViewWindow();
    glFlush();
    glFinish();

    theSDR.doDump |= theSDR.w0 && theSDR.h0 && winwidth > theSDR.w0 && winheight > theSDR.h0;
}

void GLVisDriver::getMatrix() {


    float mP[4][4];
    glGetFloatv(GL_PROJECTION_MATRIX,  mP[0]); // download projection matrix
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();                           // copy of modelview matrix on stack
    glMultMatrixf(mP[0]);                     // multiply PROJECTION*MODELVIEW
    float PVM[4][4];
    glGetFloatv(GL_MODELVIEW_MATRIX, PVM[0]); // download multiplied matrix
    glPopMatrix();                            // remove modified matrix

    // convert from column to row-major order; unscale from clipping coordinates
    for(auto i: {0,1,2,3}) {
        for(auto j: {0,1,2,3}) {
            mProj[i][j] = PVM[j][i];
            if(j<3) mProj[i][j] *= (win_hi[j] - win_lo[j])/2;
            if(j==2) mProj[i][j] *= -1;
        }
    }
}

void GLVisDriver::updateViewWindow() {
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glTranslatef(0,0,1); // viewer sits at z=+1, looking in -z direction

    win_lo[0] = win_c[0] - viewrange*ar;
    win_hi[0] = win_c[0] + viewrange*ar;
    win_lo[1] = win_c[1] - viewrange;
    win_hi[1] = win_c[1] + viewrange;
    win_lo[2] = win_c[2] - 5;
    win_hi[2] = win_c[2] + 5;
    glOrtho(win_lo[0], win_hi[0],
            win_lo[1], win_hi[1],
            win_lo[2], win_hi[2]);

    getMatrix();
}

void _keypress(unsigned char key, int x, int y) {
    if(GLDr) GLDr->keypress(key, x, y);
}

void GLVisDriver::keypress(unsigned char key, int x, int y) {
    if(key == 32 || key == 13) {  // spacebar or return
        pause_callback = &_end_pause;
        return;
    } else if(key == 27) resetViewTransformation();     // escape
    else if(pause_callback) {
        VGLCallback C;
        C.x = x;
        C.y = y;
        C.a = key;
        C.b = 0;
        C.reason = VGLCallback::KEYPRESS;
        (*pause_callback)(pause_args, &C);
    } else printf("Un-assigned keypress %u at %i,%i\n", key, x, y);
}

void GLVisDriver::startMouseTracking(int button, int state, int x, int y) {
    modifier = glutGetModifiers();
    if(state == GLUT_DOWN) {
        clickx0 = x;
        clicky0 = y;
    }

    if(pause_callback) {
        VGLCallback C;
        C.x = x;
        C.y = y;
        C.a = state;
        C.b = button;
        C.reason = VGLCallback::STARTMOUSE;
        (*pause_callback)(pause_args, &C);
    }
}

void GLVisDriver::mouseTrackingAction(int x, int y) {

    if(modifier == GLUT_ACTIVE_CTRL) {
        float s = (1 - 0.005*(x-clickx0));
        if( (viewrange > 1.0e-2 || s > 1.0) && (viewrange < 1.0e3 || s < 1.0) ) viewrange *= s;
        updateViewWindow();
        glLineWidth(1.5/viewrange);

    } else if(modifier == GLUT_ACTIVE_SHIFT) {
        win_c[0] -= ar*2.0*(x-clickx0)*viewrange/winwidth;
        win_c[1] += 2.0*(y-clicky0)*viewrange/winheight;
        updateViewWindow();
    } else {

        glMatrixMode(GL_MODELVIEW);

        float m[16];
        glGetFloatv(GL_MODELVIEW_MATRIX , m);

        if(modifier == (GLUT_ACTIVE_CTRL | GLUT_ACTIVE_SHIFT))
            glRotatef( -0.2*(x-clickx0),0,0,1);
        else {
            glRotatef( 0.2*(y-clicky0),m[0],m[4],m[8]);
            glRotatef( 0.2*(x-clickx0),m[1],m[5],m[9]);
        }

        getMatrix();
    }

    clickx0 = x; clicky0 = y;

    glFlush();
    glFinish();
    updated = true;

    if(pause_callback) {
        VGLCallback C;
        C.x = x;
        C.y = y;
        C.a = modifier;
        C.b = 0;
        C.reason = VGLCallback::MOVEMOUSE;
        (*pause_callback)(pause_args, &C);
    }
}




  /*
    void _plane(std::vector<float>& v) {
        glBegin(GL_QUADS);
        glVertex3f(v[0]+0.5*v[3]+0.5*v[6], v[1]+0.5*v[4]+0.5*v[7], v[2]+0.5*v[5]+0.5*v[8]);
        glVertex3f(v[0]-0.5*v[3]+0.5*v[6], v[1]-0.5*v[4]+0.5*v[7], v[2]-0.5*v[5]+0.5*v[8]);
        glVertex3f(v[0]-0.5*v[3]-0.5*v[6], v[1]-0.5*v[4]-0.5*v[7], v[2]-0.5*v[5]-0.5*v[8]);
        glVertex3f(v[0]+0.5*v[3]-0.5*v[6], v[1]+0.5*v[4]-0.5*v[7], v[2]+0.5*v[5]-0.5*v[8]);
        glEnd();
    }

    void plane(vec3 o, vec3 dx, vec3 dy) {
        qcmd c(_plane);
        appendv(c.v,o*scale);
        appendv(c.v,dx*scale);
        appendv(c.v,dy*scale);
        addCmd(c);
    }

    void _quad(std::vector<float>& xyz) {
        if(wireframe) glBegin(GL_LINE_LOOP);
        else glBegin(GL_QUADS);
        glVertex3f(xyz[0],xyz[1],xyz[2]);
        glVertex3f(xyz[3],xyz[4],xyz[5]);
        glVertex3f(xyz[9],xyz[10],xyz[11]);
        glVertex3f(xyz[6],xyz[7],xyz[8]);
        glEnd();
    }
    void quad(float* xyz) {
        qcmd c(_quad);
        c.v = std::vector<float>(xyz,xyz+12);
        for(size_t i = 0; i < 12; i++) c.v[i] *= scale;
        addCmd(c);
    }
    */

void _tryFlush() {
    if(kill_flag) exit(0);
    if(GLDr) GLDr->tryFlush();
    usleep(50000);
}

void _redrawDisplay() {
    if(GLDr) GLDr->redrawDisplay();
}

void _reshapeWindow(int width, int height) {
    if(GLDr) GLDr->reshapeWindow(width, height);
}

void _specialKeypress(int, int, int) { }

void _startMouseTracking(int button, int state, int x, int y) {
    if(GLDr) GLDr->startMouseTracking(button, state, x, y);
}

void _mouseTrackingAction(int x, int y) {
    if(GLDr) GLDr->mouseTrackingAction(x, y);
}


void* visthread(void*) {
    if(!GLDr) throw std::runtime_error("GLVisDriver uninitialized");

    int argc = 0;
    char* argv[] = {nullptr};
    glutInit(&argc, argv);

    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize(600, 600);
    glutInitWindowPosition(100, 100);
    glutCreateWindow(GLDr->windowTitle.c_str());
    glutDisplayFunc(&_redrawDisplay);
    glutMouseFunc(&_startMouseTracking);
    glutMotionFunc(&_mouseTrackingAction);
    glutReshapeFunc(&_reshapeWindow);
    glutKeyboardFunc(&_keypress);
    glutSpecialFunc(&_specialKeypress);
    glutIdleFunc(&_tryFlush);

    glHint(GL_LINE_SMOOTH_HINT,GL_NICEST);
    glEnable(GL_LINE_SMOOTH);
    glEnable(GL_BLEND);
    glBlendFunc(GL_ONE_MINUS_DST_ALPHA,GL_DST_ALPHA);
    glEnable(GL_DEPTH_TEST);

    glEnable(GL_FOG);

    // fade to dark
    float fadecolor[4] = {0,0,0,1.0};
    glFogfv(GL_FOG_COLOR,fadecolor);

    GLint fog_mode = GL_LINEAR;
    glFogiv(GL_FOG_MODE, &fog_mode);

    float fog_start = 2;
    float fog_end = -2;
    glFogfv(GL_FOG_START, &fog_start);
    glFogfv(GL_FOG_END, &fog_end);

    //float fog_dens = 0.7;
    //glFogfv(GL_FOG_DENSITY,&fog_dens);

    //GLint fog_coord = GL_FRAGMENT_DEPTH; //GL_FOG_COORD;
    //glFogiv(GL_FOG_COORD_SRC,&fog_coord);

    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glversion = reinterpret_cast<const char*>(glGetString(GL_VERSION));
    glutversion = glutGet(0x01FC); // GLUT_VERSION in freeglut_ext.h header

    GLDr->resetViewTransformation();
    GLDr->startRecording(true);
    GLDr->setColor(0.7, 0, 1, 0.5);
    GLDr->teapot(0.5);
    GLDr->stopRecording();

    glutIsInitialized = true;
    glutMainLoop();
    return nullptr;
}

void GLVisDriver::doGlutLoop() {
    if(GLDr) throw std::logic_error("Only one OpenGL visualizer can run at a time");
    glutIsInitialized = kill_flag = false;
    GLDr = this;
    pthread_create(&vthread, nullptr, &visthread, nullptr);
    while(!glutIsInitialized) usleep(10000);
}

#else

void GLVisDriver::doGlutLoop() { }
void GLVisDriver::endGlutLoop() { }
void GLVisDriver::pause(void (*)(void*, VGLCallback*), void*) { }

#endif
