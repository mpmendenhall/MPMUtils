/// \file SVGBuilder.hh \brief Scalable Vector Graphics XML tags
#ifndef SVGBUILDER_HH
#define SVGBUILDER_HH

#include "XMLBuilder.hh"
#include "StringManip.hh"
#include "ColorSpec.hh"

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
    };
    
    class title: public XMLBuilder {
    public:
        title(const string& t): XMLBuilder("title") { addChild(new XMLText(t)); }
    };
    
    class line: public XMLBuilder {
    public:
        line(double x1, double y1, double x2, double y2, const string& style): XMLBuilder("line") {
            attrs["x1"] = to_str(x1);
            attrs["y1"] = to_str(y1); 
            attrs["x2"] = to_str(x2); 
            attrs["y2"] = to_str(y2);
            if(style.size()) attrs["style"] = style;
        }
    };
    
    class polyline: public XMLBuilder {
    public:
        polyline(const string& style): XMLBuilder("polyline") { if(style.size()) attrs["style"] = style; }
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
        polygon(const string& style): polyline(style) { name = "polygon"; }
    };
}

#endif
