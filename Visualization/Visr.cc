/// \file Visr.cc
// Michael P. Mendenhall, 2018

#include "Visr.hh"
#include "GeomCalcUtils.hh"
#include <stdio.h>
#include <cassert>
#include <string>
#include <unistd.h>

namespace vsr {

    vec3 operator*(const vec3& v, double s) { return vec3{v[0]*s,v[1]*s,v[2]*s}; }

    struct qcmd {
        qcmd(void (*f)(std::vector<float>&)): fcn(f) {}
        void (*fcn)(std::vector<float>&);
        std::vector<float> v;
    };
    std::deque<qcmd> commands;
    pthread_mutex_t commandLock;
    float scale;
    int clickx0,clicky0;
    int modifier;
    int winwidth, winheight;
    float viewrange;
    float xrot,yrot,zrot;
    float xtrans, ytrans;
    float ar;
    bool pause_display;
    bool wireframe = true;
    bool kill_flag = false;
    float bgR, bgG, bgB, bgA;
    bool window_open = false;

    void addCmd(qcmd c) {
        if(!window_open) return;
        pthread_mutex_lock(&commandLock);
        commands.push_back(c);
        pthread_mutex_unlock(&commandLock);
    }
    void appendv(std::vector<float>& v, vec3 a) {
        v.push_back(a[0]);
        v.push_back(a[1]);
        v.push_back(a[2]);
    }

    void set_pause() { pause_display = true; }
    bool get_pause() { return pause_display; }
    void set_kill() { kill_flag = true; }
    void _wireframe(std::vector<float>&) { wireframe = true; }
    void _solid(std::vector<float>&) { wireframe = false; }
    void setWireframe(bool w) { addCmd(w? _wireframe : _solid); }

    void setClearColor(float r, float g, float b, float a) {
        bgR = r;
        bgG = g;
        bgB = b;
        bgA = a;
    }

    bool glutLooping = false;
}



#ifdef WITH_OPENGL
#include <GL/freeglut.h>

bool Visualizable::vis_on = true;

namespace vsr {

    GLuint displayList;
    std::vector<GLuint> displaySegs;

    void doGlutLoop() {
        glutLooping = true;
        glutMainLoop();
        glutLooping = false;
    }

    void redrawDisplay() {
        if(!displaySegs.size()) return;

        glCallLists(displaySegs.size(), GL_UNSIGNED_INT, &displaySegs.front());
        glutSwapBuffers();
        glFlush();
        glFinish();
    }

    void resetViewTransformation();
    void reshapeWindow(int width, int height);
    void keypress(unsigned char key, int x, int y);
    void specialKeypress(int key, int x, int y);
    void startMouseTracking(int button, int state, int x, int y);
    void mouseTrackingAction(int x, int y);
    void redrawIfUnlocked();

    void _setColor(std::vector<float>& v) {
        assert(v.size()==4);
        glColor4f(v[0],v[1],v[2],v[3]);
    }
    void setColor(float r, float g, float b, float a) {
        qcmd c(&_setColor);
        c.v.push_back(r);
        c.v.push_back(g);
        c.v.push_back(b);
        c.v.push_back(a);
        addCmd(c);
    }

    void _clearWindow(std::vector<float>&) {
        glClearColor(bgR, bgG, bgB, bgA);
        glClearDepth(100.0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    }
    void clearWindow() {
        addCmd(qcmd(_clearWindow));
    }

    void pause() {
        if(!Visualizable::vis_on || !window_open) return;
        pause_display = true;
        printf("Press [enter] in visualization window to continue...\n");
        while(pause_display) usleep(50000);
    }

    void resetViewTransformation() {
        //printf("Re-setting view...\n");
        viewrange = 1.0;
        xtrans = ytrans = 0;
        glLineWidth(1.5/viewrange);
        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();
        glTranslatef(0, 0, 1.0*viewrange);
        updateViewWindow();
    }

    void _startRecording(std::vector<float>& v) {
        glFlush();
        glFinish();

        if(v.size()) { // "newseg" overwrite/append
            while(displaySegs.size()) {
                if(glIsList(displaySegs.back())) glDeleteLists(displaySegs.back(), 1);
                displaySegs.pop_back();
            }
        }
        displaySegs.push_back(glGenLists(1));
        glNewList(displaySegs.back(), GL_COMPILE);
    }
    void startRecording(bool newseg) {
        if(!window_open) return;
        pthread_mutex_lock(&commandLock);
        if(newseg) commands.clear();
        qcmd c(_startRecording);
        if(newseg) c.v.push_back(1); // mark as addition to previous segment
        addCmd(c);
    }

    void _stopRecording(std::vector<float>&) {
        glEndList();
        glutPostRedisplay();
        glFlush();
        glFinish();
    }
    void stopRecording() {
        if(!window_open) return;
        addCmd(qcmd(_stopRecording));
        pthread_mutex_unlock(&commandLock);
    }

    void screendump(const char* fname) {
        if(!window_open || !fname) return;
        pthread_mutex_lock(&commandLock);

        printf("Saving %i x %i screendump to '%s'\n", winwidth, winheight, fname);
        auto fout = fopen(fname,"wb");

        std::vector<char> pbuff(3*winwidth*winheight);
        glGetError();
        glReadBuffer(GL_FRONT);
        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
        glPixelStorei(GL_PACK_ALIGNMENT, 1);
        glReadPixels(0, 0, winwidth, winheight, GL_BGR, GL_UNSIGNED_BYTE, pbuff.data());
        assert(glGetError() == GL_NO_ERROR);

        short fhead[] = { 0, 2, 0, 0, 0, 0, short(winwidth), short(winheight), 24 }; // .tga file header
        fwrite(fhead, sizeof(fhead), 1, fout);
        fwrite(pbuff.data(), pbuff.size(), 1, fout);
        fclose(fout);

        pthread_mutex_unlock(&commandLock);
    }

    void _line(std::vector<float>& v) {
        assert(v.size()==6);
        glBegin(GL_LINES);
        glVertex3f(v[0],v[1],v[2]);
        glVertex3f(v[3],v[4],v[5]);
        glEnd();
    }
    void line(vec3 s, vec3 e) {
        qcmd c(_line);
        appendv(c.v,s*scale);
        appendv(c.v,e*scale);
        addCmd(c);
    }

    void _startLines(std::vector<float>&) {
        glBegin(GL_LINE_STRIP);
    }
    void startLines() {
        addCmd(qcmd(_startLines));
    }
    void _vertex(std::vector<float>& v) {
        glVertex3f(v[0],v[1],v[2]);
    }
    void vertex(vec3 v) {
        qcmd c(_vertex);
        appendv(c.v,v*scale);
        addCmd(c);
    }

    void _endLines(std::vector<float>&) {
        glEnd();
    }
    void endLines() {
        addCmd(qcmd(_endLines));
    }

    void _plane(std::vector<float>& v) {
        assert(v.size()==9);
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

    void _polyline(std::vector<float>& v) {
        glBegin(GL_LINE_LOOP);
        for(unsigned int i=0; i+2<v.size(); i += 3) glVertex3f(v[i],v[i+1],v[i+2]);
        glEnd();
    }

    void circle(vec3 o, vec3 n, int i) {
        int j0 = 0;
        for(auto j: {1,2}) if(fabs(n[j]) > fabs(n[j0])) j0 = j;
        vec3 dz{};
        dz[(j0+1)%3] = 1;

        auto r = makeunit(n);
        r *= scale;
        vec3 dx;
        vec3 dy;
        ortho_frame(dz, n, dx, dy);

        qcmd cp(_polyline);
        for(int p=0; p<i; p++) {
            double th = p*2*M_PI/i;
            double c = r*cos(th);
            double s = r*sin(th);
            for(auto j: {0,1,2}) cp.v.push_back(o[j]*scale+c*dx[j]+s*dy[j]);
        }
        addCmd(cp);
    }

    void _quad(std::vector<float>& xyz) {
        assert(xyz.size()==12);
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

    void _ball(std::vector<float>& v) {
        glMatrixMode(GL_MODELVIEW);
        glPushMatrix();
        glTranslatef(v[0],v[1],v[2]);
        if(wireframe) glutWireSphere(v[3], v[4], v[5]);
        else glutSolidSphere(v[3], v[4], v[5]);
        glPopMatrix();
    }
    void ball(vec3 p, double r, int nx, int ny) {
        qcmd c(_ball);
        appendv(c.v, p*scale);
        c.v.push_back(r*scale);
        c.v.push_back(nx);
        c.v.push_back(ny);
        addCmd(c);
    }

    void _dot(std::vector<float>& p) {
        assert(p.size()==3);
        double l = viewrange/100.0;
        glBegin(GL_QUADS);
        glVertex3f(p[0]+l,p[1],p[2]);
        glVertex3f(p[0],p[1]+l,p[2]);
        glVertex3f(p[0]-l,p[1],p[2]);
        glVertex3f(p[0],p[1]-l,p[2]);
        glVertex3f(p[0]+l,p[1],p[2]);
        glVertex3f(p[0],p[1],p[2]+l);
        glVertex3f(p[0]-l,p[1],p[2]);
        glVertex3f(p[0],p[1],p[2]-l);
        glVertex3f(p[0],p[1]+l,p[2]);
        glVertex3f(p[0],p[1],p[2]+l);
        glVertex3f(p[0],p[1]-l,p[2]);
        glVertex3f(p[0],p[1],p[2]-l);
        glEnd();
    }
    void dot(vec3 v) {
        qcmd c(_dot);
        appendv(c.v, v*scale);
        addCmd(c);
    }

    void _teapot(std::vector<float>& v) {
        if(wireframe) glutWireTeapot(v[0]);
        else glutSolidTeapot(v[0]);
    }
    void teapot(double s) {
        qcmd c(_teapot);
        c.v.push_back(s);
        addCmd(c);
    }

    void initWindow(const std::string& windowTitle, double s) {
        scale = s;

        pause_display = false;
        pthread_mutexattr_t displayLockAttr;
        pthread_mutexattr_init(&displayLockAttr);
        pthread_mutexattr_settype(&displayLockAttr, PTHREAD_MUTEX_RECURSIVE);
        pthread_mutex_init(&commandLock,&displayLockAttr);

        int a = 1;
        char programname[] = "glviewer";
        char* pnameptr = programname;
        glutInit(&a,&pnameptr);
        glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
        glutInitWindowSize(600, 600);
        glutInitWindowPosition(100, 100);
        glutCreateWindow(windowTitle.c_str());

        ar = 1.0;

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

        glutDisplayFunc(&redrawDisplay);
        glutMouseFunc(&startMouseTracking);
        glutMotionFunc(&mouseTrackingAction);
        glutReshapeFunc(&reshapeWindow);
        glutKeyboardFunc(&keypress);
        glutSpecialFunc(&specialKeypress);
        glutIdleFunc(&redrawIfUnlocked);

        window_open = true;

        resetViewTransformation();
        setClearColor(1.0,1.0,1.0,0.0);

        startRecording(true);
        clearWindow();
        glColor3f(0.0, 0.0, 1.0);
        glutWireTeapot(0.5);
        stopRecording();
    }

    void reshapeWindow(int width, int height) {
        glViewport(0,0,width,height);
        winwidth = width; winheight = height;
        ar = float(width)/float(height);
        updateViewWindow();
        glFlush();
        glFinish();
    }

    void updateViewWindow() {
        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
        glTranslatef(0,0,1); // viewer sits at z=+1, looking in -z direction
        //      left,                   right,                 bottom,              top,                distance to near, distance to far
        glOrtho(-viewrange*ar + xtrans, viewrange*ar + xtrans, -viewrange + ytrans, viewrange + ytrans, 0,                10);
    }

    void redrawIfUnlocked() {
        if(kill_flag) exit(0);
        if(pthread_mutex_trylock(&commandLock)) return;
        while(commands.size()) {
            auto f = commands.front().fcn;
            if(f) f(commands.front().v);
            commands.pop_front();
        }
        redrawDisplay();
        pthread_mutex_unlock(&commandLock);
        usleep(10000);
    }

    void keypress(unsigned char key, int x, int y) {
        if(key == 32 || key == 13) pause_display = false; // spacebar or return
        else if(key == 27) resetViewTransformation();     // escape
        else if(key == 100) { // 'd'
            static int ndumps = 0;
            char dname[1024];
            sprintf(dname, "screendump_%03i.tga", ndumps++);
            screendump(dname);
        }
        else printf("Un-assigned keypress %u at %i,%i\n", key, x, y);
    }

    void specialKeypress(int, int, int) {

    }

    void startMouseTracking(int, int state, int x, int y) {
        modifier = glutGetModifiers();
        if(state == GLUT_DOWN) {
            clickx0 = x;
            clicky0 = y;
        }
    }

    void mouseTrackingAction(int x, int y) {

        if(modifier == GLUT_ACTIVE_SHIFT) {
            float s = (1 - 0.005*(x-clickx0));
            if( (viewrange > 1.0e-2 || s > 1.0) && (viewrange < 1.0e3 || s < 1.0) ) viewrange *= s;
            updateViewWindow();
            glLineWidth(1.5/viewrange);

        } else if(modifier == GLUT_ACTIVE_CTRL) {
            xtrans -= ar*2.0*(x-clickx0)*viewrange/winwidth;
            ytrans += 2.0*(y-clicky0)*viewrange/winheight;
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
        }

        clickx0 = x; clicky0 = y;

        glFlush();
        glFinish();
    }
}

#else

bool Visualizable::vis_on = false;

namespace vsr {

    void initWindow(const std::string&, double) { }
    void clearWindow() {}
    void resetViewTransformation() {}
    void updateViewWindow() {}
    void startRecording(bool) {}
    void stopRecording() {}
    void pause() {}
    void setColor(float, float, float, float) {}
    void line(vec3, vec3) {}
    void plane(vec3, vec3, vec3) {}
    void quad(float*) {}
    void dot(vec3) {}
    void ball(vec3, double, int, int) { }
    void circle(vec3, vec3, int) { }
    void teapot(double) { }
    void screendump(const char*) { }

    void startLines() {}
    void vertex(vec3) {}
    void endLines() {}

    void doGlutLoop() {}
}

#endif

