/// \file SVGBuilder.hh Scalable Vector Graphics XML tags

// This file was produced under the employ of the United States Government,
// and is consequently in the PUBLIC DOMAIN, free from all provisions of
// US Copyright Law (per USC Title 17, Section 105).
//
// -- Michael P. Mendenhall, 2015

#ifndef SVGBUILDER_HH
#define SVGBUILDER_HH

#include "XMLBuilder.hh"
#include "StringManip.hh"
#include "ColorSpec.hh"
#include "BBox.hh"
#include <cmath>

namespace SVG {

    class svg: public XMLBuilder {
    public:
        svg(): XMLBuilder("svg") {
            attrs["version"] = "1.1";
            attrs["xmlns"] = "http://www.w3.org/2000/svg";
            attrs["xmlns:xlink"] = "http://www.w3.org/1999/xlink";
        }

        void setView(BBox<2,double> BB, double xToCm) {
            attrs["viewBox"] = to_str(BB.lo[0])+","+to_str(BB.lo[1])+","+to_str(BB.dl(0))+","+to_str(BB.dl(1));
            attrs["width"] = to_str(BB.dl(0)*xToCm)+"cm";
            attrs["height"] = to_str(BB.dl(1)*xToCm)+"cm";
        }

        static  void make_standalone_header(ostream& o) {
            o << "<?xml version=\"1.0\" standalone=\"no\"?>\n";
            o << "<!DOCTYPE svg PUBLIC \"-//W3C//DTD SVG 1.1//EN\" \"http://www.w3.org/Graphics/SVG/1.1/DTD/svg11.dtd\">\n";
        }
    };

    class group: public XMLBuilder {
    public:
        group(): XMLBuilder("g") { }
    };

    class defs: public XMLBuilder {
    public:
        defs(): XMLBuilder("defs") { }
    };

    class title: public XMLBuilder {
    public:
        title(const string& t): XMLBuilder("title") { addChild(make_shared<XMLText>(t)); oneline = true; }
    };

    class line: public XMLBuilder {
    public:
        line(double x1, double y1, double x2, double y2, const string& style = ""): XMLBuilder("line") {
            addAttr("x1",x1);
            addAttr("y1",y1);
            addAttr("x2",x2);
            addAttr("y2",y2);
            if(style.size()) attrs["style"] = style;
        }
    };

    class rect: public XMLBuilder {
    public:
        rect(double x, double y, double dx, double dy, const string& style = ""): XMLBuilder("rect") {
            if(dx < 0) { x += dx; dx = fabs(dx); }
            if(dy < 0) { y += dy; dy = fabs(dy); }
            addAttr("x",x);
            addAttr("y",y);
            addAttr("width",dx);
            addAttr("height",dy);
            if(style.size()) attrs["style"] = style;
        }
    };

    class circle: public XMLBuilder {
    public:
        circle(double cx, double cy, double r, const string& style = ""): XMLBuilder("circle") {
            addAttr("cx",cx);
            addAttr("cy",cy);
            addAttr("r",r);
            if(style.size()) attrs["style"] = style;
        }
    };
    
    class ellipse: public XMLBuilder {
    public:
        ellipse(double cx, double cy, double rx, double ry, const string& style = ""): XMLBuilder("ellipse") {
            addAttr("cx",cx);
            addAttr("cy",cy);
            addAttr("rx",rx);
            addAttr("rx",ry);
            if(style.size()) attrs["style"] = style;
        }
    };
    
    class polyline: public XMLBuilder {
    public:
        polyline(const string& style = ""): XMLBuilder("polyline") { if(style.size()) attrs["style"] = style; }
        void addpt(double x, double y) { pts.push_back(pair<double,double>(x,y)); }
        vector< pair<double,double> > pts;
    protected:
        virtual void prepare() {
            string s = "";
            for(auto const& pt: pts) s += to_str(pt.first) + "," +  to_str(pt.second) + " ";
            attrs["points"] = s;
        }
    };

    class polygon: public polyline {
    public:
        polygon(const string& style = ""): polyline(style) { name = "polygon"; }
    };


    class gradstop: public XMLBuilder {
    public:
        gradstop(double l, color::rgb c): XMLBuilder("stop") {
            addAttr("offset",l);
            attrs["stop-color"] = "#"+c.asHexString();
            if(c.a != 1) addAttr("stop-opacity",c.a);
        }
    };

    class lingradient: public XMLBuilder {
    public:
        lingradient(const color::Gradient& G, const string& id, double x1, double y1, double x2, double y2): XMLBuilder("linearGradient") {
            attrs["id"] = id;
            addAttr("x1",x1);
            addAttr("y1",y1);
            addAttr("x2",x2);
            addAttr("y2",y2);
            for(auto const& s: G.getStops()) addChild(make_shared<gradstop>(s.first, s.second.first));
        }

        string idstr() const { return "url(#" + attrs.find("id")->second + ")"; }
    };

    class text: public XMLBuilder {
    public:
        text(const string& t, double x, double y, const string& fill = "black"): XMLBuilder("text"), myText(make_shared<XMLText>(t)) {
            addAttr("x",x);
            addAttr("y",y);
            attrs["fill"] = fill;
            oneline = true;
            addChild(myText);
        }
        shared_ptr<XMLText> myText;
    };
    
    inline void set_fill(shared_ptr<XMLBuilder> x, const color::rgb& c) {
        x->attrs["fill"] = "#"+c.asHexString();
        if(c.a != 1) x->addAttr("fill-opacity",c.a);
    }
}

#endif
