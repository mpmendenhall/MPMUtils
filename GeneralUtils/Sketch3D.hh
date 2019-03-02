/// \file Sketch3D.hh SVG-based 3D-to-2D projection "sketching" utilities
// Michael P. Mendenhall, 2017

#ifndef SKETCH3D_HH
#define SKETCH3D_HH

#include "SVGBuilder.hh"
#include <cassert>
#include <memory>
using std::unique_ptr;

/// replacement for c++14 make_unique (missing in c++11)
template<typename T, typename... Args>
unique_ptr<T> make_unique(Args&&... args) {
    return unique_ptr<T>(new T(std::forward<Args>(args)...));
}

/// convenience typedef for 3D point
typedef std::array<double,3> xyzpt;
/// convenient typedef for perspective-scaled 3D point
typedef std::array<double,4> xyzspt;

/// Perspecictive projection model
class Perspective {
public:
    /// Constructor
    Perspective() { clearRotation(); }

    /// project point from 3D x,y,z to 2D x,y,z + perspective scale factor
    void project(const double xyz[3], double xyzs[4]) const;
    /// project point from 3D x,y,z to 2D x,y,z + perspective scale factor
    void project(const xyzpt& xyz, xyzspt& xyzs) const { project(xyz.data(), xyzs.data()); }
    /// set identity rotation
    void clearRotation();

    bool isOrtho = true;    /// orthographic projection mode
    bool flipY = true;      /// y "flip" for +y=down drawing coordinates
    double M[3][3];         /// model points rotation matrix
    double v0[3] = {0,0,0}; /// viewer position

    /// append list of output points to projection vector; return average z and scale factor
    void projectPoly(const vector<xyzpt>& vIn, vector<SVG::xypoint>& vOut, double& s, double& z) const;
};

/// Base for generating z-orderable projected SVG primitives
class ProjectablePrimitive {
public:
    /// Generate XML and calculate z for perspective
    virtual void setPerspective(const Perspective& P) = 0;

    std::shared_ptr<SVGBuilder> myXML;   ///< generated XML
    double z0 = 0;                  ///< depth-sorting shift for all projections
    double z = 0;                   ///< depth-sorting z in projected state
    double s = 0;                   ///< overall scale factor
    map<string,string> attrs;       ///< non-scaled attributes
    map<string,double> sattrs;      ///< helper list of perspective-scaled attributes
protected:
    /// apply (scaled) attributes
    void setAttrs() {
        assert(myXML);
        for(auto& kv: attrs) myXML->addAttr(kv.first, kv.second);
        for(auto& kv: sattrs) myXML->addAttr(kv.first, s*kv.second);
    }
};

/// 3D-projectable (pointlike) ball/circle
class ProjectableBall: public ProjectablePrimitive {
public:
    /// Constructor
    ProjectableBall(xyzpt cc, double rr): c(cc), r(rr) { }

    /// Generate XML and calculate z for perspective
    void setPerspective(const Perspective& P) override;

    xyzpt c;    ///< center
    double r;   ///< radius
};

/// 3D-projectable polyline/polygon
class ProjectablePoly: public ProjectablePrimitive {
public:
    /// Generate XML and calculate z for perspective
    void setPerspective(const Perspective& P) override;

    bool closed = false;    ///< closed (polygon) or open (polyline) curve
    vector<xyzpt> pts;      ///< points on line
};


/// Base class generator for one "layer" of 3D-projected drawing
class SketchLayer {
public:
    /// "Draw" contents into provided parent using projection
    virtual void drawInto(XMLBuilder& X, const Perspective& P) = 0;
    /// Render contents to file
    void makeSVG(const Perspective& P, const string& fname, double xborder = 0, const string& ttl="");
    /// Render contents to stereo pair
    void makeStereo(Perspective& P, const string& fname, double xborder = 0, const string& ttl="");
};

/// Composite of multiple SVG sketch layers
class MultiLayer: public SketchLayer {
public:
    /// "Draw" contents into provided parent using projection
    void drawInto(XMLBuilder& X, const Perspective& P) override;

    vector<SketchLayer*> myLayers;  /// layers, back to front
};

/// Layer with z-sortable list of primitives
class PrimitivesLayer: public SketchLayer {
public:
    /// "Draw" contents into provided parent using projection
    void drawInto(XMLBuilder& X, const Perspective& P) override;

    vector<unique_ptr<ProjectablePrimitive>> myObjs; ///< drawable objects
};

/*
/// Sketch in transformed 2D plane
class PlaneLayer: public SketchLayer {
public:
    /// "Draw" contents into provided parent using projection
    void drawInto(XMLBuilder& X, const Perspective& P) override {
        auto gg = make_shared<group>(g);
        gg->attrs["transform"] = "matrix()"; // TODO
        X.addChild(gg);
    }

    group g;                    ///< 2D drawing group to be transformed
    double o[3] = {0,0,0};      ///< plane origin (untransformed)
    double M[3][3];             ///< plane unit vectors vx,vy,vz
};
*/

#endif
