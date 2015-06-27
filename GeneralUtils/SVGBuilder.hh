/// \file SVGBuilder.hh \brief Scalable Vector Graphics XML tags

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

namespace SVG {

    void make_standalone_header(ostream& o) {
        o << "<?xml version=\"1.0\" standalone=\"no\"?>\n";
        o << "<!DOCTYPE svg PUBLIC \"-//W3C//DTD SVG 1.1//EN\" \"http://www.w3.org/Graphics/SVG/1.1/DTD/svg11.dtd\">\n";
    }
    
    class svg: public XMLBuilder {
    public:
        svg(): XMLBuilder("svg") {
            attrs["version"]="1.1";
            attrs["xmlns"]="http://www.w3.org/2000/svg";
        }
        
        void setView(BBox<2,double> BB, double xToCm) {
            attrs["viewBox"] = to_str(BB.lo[0])+","+to_str(BB.lo[1])+","+to_str(BB.dl(0))+","+to_str(BB.dl(1));
            attrs["width"] = to_str(BB.dl(0)*xToCm)+"cm";
            attrs["height"] = to_str(BB.dl(1)*xToCm)+"cm";
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
        title(const string& t): XMLBuilder("title") { addChild(new XMLText(t)); }
    };
    
    class line: public XMLBuilder {
    public:
        line(double x1, double y1, double x2, double y2, const string& style = ""): XMLBuilder("line") {
            attrs["x1"] = to_str(x1);
            attrs["y1"] = to_str(y1); 
            attrs["x2"] = to_str(x2); 
            attrs["y2"] = to_str(y2);
            if(style.size()) attrs["style"] = style;
        }
    };
    
    class rect: public XMLBuilder {
    public:
        rect(double x, double y, double dx, double dy, const string& style = ""): XMLBuilder("rect") {
            attrs["x"] = to_str(x);
            attrs["y"] = to_str(y);
            attrs["width"] = to_str(dx);
            attrs["height"] = to_str(dy);
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
            for(auto it = pts.begin(); it != pts.end(); it++)
                s += to_str(it->first) + "," +  to_str(it->second) + " ";
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
            attrs["offset"] = to_str(l);
            attrs["style"] = "stop-color:#"+c.asHexString() + (c.a != 1? ";stop-opacity:"+to_str(c.a) : "");
        }
    };
    
    class lingradient: public XMLBuilder {
    public:
        lingradient(const color::Gradient& G, const string& id, double x1, double y1, double x2, double y2): XMLBuilder("linearGradient") {
            attrs["id"] = id;
            attrs["x1"] = to_str(x1);
            attrs["y1"] = to_str(y1);
            attrs["x2"] = to_str(x2);
            attrs["y2"] = to_str(y2);
            for(auto it = G.getStops().begin(); it != G.getStops().end(); it++)
                addChild(new gradstop(it->first, it->second.first));
        }
        
        string idstr() const { return "url(#" + attrs.find("id")->second + ")"; }
    };
}

#endif
