/// \file Visr.hh Simple OpenGL visualization window
/*
 * Visr.hh, part of the MPMUtils package
 * Copyright (c) 2007-2014 Michael P. Mendenhall
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */

#ifndef VISCONTROLLER_HH
/// Make sure this header is only loaded once
#define VISCONTROLLER_HH

#include <vector>
#include <deque>
#include <array>

namespace vsr {

    typedef std::array<double,3> vec3;

    /// initialize visualization window
    void initWindow(const std::string& title = "OpenGL Viewer Window", double s = 1.0);
    /// enter main drawing loop
    void doGlutLoop();

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

    /// clear window to blank screen
    void clearWindow();
    /// set background clear color
    void setClearColor(float r, float g, float b, float a=0);
    /// set color for subsequent draws
    void setColor(float r, float g, float b, float a = 1);
    /// set solid/wireframe mode
    void setWireframe(bool w);
    /// draw specified line
    void line(vec3 s, vec3 e);
    /// draw specified plane, centered at o, +/- x and y vectors
    void plane(vec3 o, vec3 dx, vec3 dy);
    /// draw quadrangle (wireframe or filled)
    void quad(float* xyz);
    /// draw dot at location
    void dot(vec3 p);
    /// draw ball at location
    void ball(vec3 p, double r, int nx = 8, int ny = 8);
    /// draw circle (polygon) with center o, normal/radius n; i line segments
    void circle(vec3 o, vec3 n, int i = 36);
    /// draw teapot
    void teapot(double s = 1.0);

    /// start a polygon/series-of-lines
    void startLines();
    /// next vertex in line series
    void vertex(vec3 v);
    /// end series of lines
    void endLines();

    /// set the pause flag, cleared when [ENTER] pressed in vis window
    void set_pause();
    /// get current state of pause flag
    bool get_pause();
    /// set the kill flag to end visualization thread
    void set_kill();
}

/// virtual base class for visualizable objects
class Visualizable {
public:
    /// constructor
    Visualizable() {}
    /// destructor
    virtual ~Visualizable() {}


    /// visualize "top level"
    void visualize() const { vsr::startRecording(true); vsr::clearWindow(); _visualize(); vsr::stopRecording(); }

    /// visualize without clearing screen
    virtual void _visualize() const = 0;

    static bool vis_on;
};


#endif
