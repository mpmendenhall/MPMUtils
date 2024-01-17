/// @file SVGBuilder.hh Scalable Vector Graphics XML tags
// -- Michael P. Mendenhall, 2017

#ifndef SVGBUILDER_HH
#define SVGBUILDER_HH

#include "XMLTag.hh"
#include "ColorSpec.hh"
#include "BBox.hh"
#include "NoCopy.hh"
#include <fstream>
using std::ofstream;
#include <cmath>

namespace SVG {

    /// convenience typedef for 2D point
    typedef std::array<double,2> xypoint;

    /// XML builder object with a bounding box calculation
    class BBXML: public XMLTag {
    public:
        /// Convenience typedef for 2D bounding boxes
        typedef BBox<2,double> BBox2;
        /// Constructor
        explicit BBXML(const string& nm): XMLTag(nm) { }
        /// Get (override to calculate) bounding box
        virtual BBox2 getBB() { return BB; }
    protected:
        /// Contents bounding box
        BBox2 BB;
        /// Calculate BBox from children
        void calcChildrenBB() {
            BB = BBox2();
            for(auto& c: children) {
                auto cc = dynamic_cast<BBXML*>(c);
                if(cc) BB += cc->getBB();
            }
        }
    };

    class svg: public BBXML {
    public:
        svg(): BBXML("svg") {
            attrs["version"] = "1.1";
            attrs["xmlns"] = "http://www.w3.org/2000/svg";
            attrs["xmlns:xlink"] = "http://www.w3.org/1999/xlink";
        }

        void setView(BBox2 BV, double xToCm) {
            attrs["viewBox"] = to_str(BV.lo[0])+","+to_str(BV.lo[1])+","+to_str(BV.dl(0))+","+to_str(BV.dl(1));
            attrs["width"] = to_str(BV.dl(0)*xToCm)+"cm";
            attrs["height"] = to_str(BV.dl(1)*xToCm)+"cm";
        }

        static  void make_standalone_header(ostream& o) {
            o << "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"no\"?>\n";
            o << "<!DOCTYPE svg PUBLIC \"-//W3C//DTD SVG 1.1//EN\" \"http://www.w3.org/Graphics/SVG/1.1/DTD/svg11.dtd\">\n";
        }

        /// Calculate bounding box from contents
        BBox2 getBB() override { calcChildrenBB(); return BB; }
    };

    class group: public BBXML {
    public:
        group(): BBXML("g") { }
        /// Calculate bounding box from contents
        BBox2 getBB() override {
            calcChildrenBB();
            if(BB.isNull()) return BB;
            for(auto i:{0,1}) { BB.lo[i] *= scale[i]; BB.hi[i] *= scale[i]; }
            BB.offset(translation);
            return BB;
        }
        xypoint translation{{0.,0.}};   ///< offset translation of element
        xypoint scale{{1.,1.}};         ///< transform re-scaling
    protected:
        void prepare() override {
            string s = "";
            if(translation[0] || translation[1]) {
                s += "translate("+to_str(translation[0]);
                if(translation[1]) s += ","+to_str(translation[1]);
                s+=")";
            }
            if(scale[0] != 1 || scale[1] != 1) {
                if(s.size()) s += " ";
                s += "scale("+to_str(scale[0]);
                if(scale[1] != scale[0]) s += ","+to_str(scale[1]);
                s+=")";
            }
            if(s.size()) attrs["transform"] = s;
        }
    };

    class defs: public XMLTag {
    public:
        defs(): XMLTag("defs") { }
    };

    class title: public XMLTag {
    public:
        explicit title(const string& t): XMLTag("title") { addChild(new XMLText(t)); oneline = true; }
    };

    class line: public BBXML {
    public:
        line(double x1, double y1, double x2, double y2, const string& style = ""): BBXML("line") {
            addAttr("x1",x1);
            addAttr("y1",y1);
            addAttr("x2",x2);
            addAttr("y2",y2);
            if(style.size()) attrs["style"] = style;

            BB.expand({{x1,y1}});
            BB.expand({{x2,y2}});
        }
    };

    class rect: public BBXML {
    public:
        rect(double x, double y, double dx, double dy, const string& style = ""): BBXML("rect") {
            if(dx < 0) { x += dx; dx = fabs(dx); }
            if(dy < 0) { y += dy; dy = fabs(dy); }
            if(style.size()) attrs["style"] = style;
            BB.expand({{x,y}});
            BB.expand({{x+dx,y+dy}});
        }
        explicit rect(BBox2 B, const string& style = ""): BBXML("rect") {
            if(style.size()) attrs["style"] = style;
            BB = B;
        }
        void setBB(BBox2 B) { BB = B; }
        void prepare() override {
            BBXML::prepare();
            addAttr("x", BB.pos(0,0));
            addAttr("y", BB.pos(0,1));
            addAttr("width", BB.dl(0));
            addAttr("height", BB.dl(1));
        }
    };

    class circle: public BBXML {
    public:
        circle(double cx, double cy, double r, const string& style = ""): BBXML("circle") {
            addAttr("cx",cx);
            addAttr("cy",cy);
            addAttr("r",r);
            if(style.size()) attrs["style"] = style;

            BB.expand({{cx-r,cy-r}});
            BB.expand({{cx+r,cy+r}});
        }
    };

    class ellipse: public BBXML {
    public:
        ellipse(double cx, double cy, double rx, double ry, const string& style = ""): BBXML("ellipse") {
            addAttr("cx",cx);
            addAttr("cy",cy);
            addAttr("rx",rx);
            addAttr("rx",ry);
            if(style.size()) attrs["style"] = style;

            BB.expand({{cx-rx,cy-ry}});
            BB.expand({{cx+rx,cy+ry}});
        }
    };

    class polyline: public BBXML {
    public:
        explicit polyline(const string& style = ""): BBXML("polyline") { if(style.size()) attrs["style"] = style; }
        void addpt(double x, double y) { pts.push_back({{x,y}}); }
        vector<xypoint> pts;
        /// Calculate bounding box from contents
        BBox2 getBB() override {
            BB = BBox2();
            for(auto p: pts) BB.expand(p);
            return BB;
        }
    protected:
        void prepare() override {
            string s = "";
            for(auto const& pt: pts) s += to_str(pt[0]) + "," +  to_str(pt[1]) + " ";
            attrs["points"] = s;
        }
    };

    class polygon: public polyline {
    public:
        explicit polygon(const string& style = ""): polyline(style) { name = "polygon"; }
    };


    class gradstop: public XMLTag {
    public:
        gradstop(double l, color::rgb c): XMLTag("stop") {
            addAttr("offset",l);
            attrs["stop-color"] = "#" + string(c);
            if(c.a != 1) addAttr("stop-opacity",c.a);
        }
    };

    class lingradient: public XMLTag {
    public:
        lingradient(const color::Gradient& G, const string& id, double x1, double y1, double x2, double y2): XMLTag("linearGradient") {
            attrs["id"] = id;
            addAttr("x1",x1);
            addAttr("y1",y1);
            addAttr("x2",x2);
            addAttr("y2",y2);
            for(auto const& s: G) addChild(new gradstop(s.first, s.second.first));
        }

        string idstr() const { return "url(#" + attrs.find("id")->second + ")"; }
    };

    class text: public BBXML, protected NoCopy {
    public:
        text(const string& t, double x, double y, const string& fill = "black"): BBXML("text"), myText(new XMLText(t)) {
            addAttr("x",x);
            addAttr("y",y);
            attrs["fill"] = fill;
            oneline = true;
            addChild(myText);
            BB.expand({{x,y}});
        }
        XMLText* myText;
    };

    /// set fill color
    inline void set_fill(map<string,string>& attrs, const color::rgb& c) {
        attrs["fill"] = "#" + string(c);
        if(c.a != 1) attrs["fill-opacity"] = to_str(c.a);
    }
    /// set fill color
    inline void set_fill(XMLTag& X, const color::rgb& c) { set_fill(X.attrs, c); }
    /// set stroke color
    inline void set_stroke(map<string,string>& attrs, const color::rgb& c) {
        attrs["stroke"] = "#" + string(c);
        if(c.a != 1) attrs["stroke-opacity"] = to_str(c.a);
    }
    /// set stroke color
    inline void set_stroke(XMLTag& X, const color::rgb& c) { set_stroke(X.attrs, c); }

    /// SVG document outline convenience class
    class SVGDoc {
    public:
        svg body;           ///< main body element
        BBXML::BBox2 BB;    ///< view bounding box
        double x2cm = 1.;   ///< output scale factor

        /// write to ostream
        void write(std::ostream& o) {
            svg::make_standalone_header(o);
            body.setView(BB, x2cm);
            body.write(o,0,"\t");
        }
        /// write to file
        void write(const string& fname) {
            ofstream o;
            o.open(fname);
            write(o);
            o.close();
        }
    };
}

#endif
