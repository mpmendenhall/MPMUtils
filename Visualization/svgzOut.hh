/// \file svgzOut.hh Helper to write .svgz compressed svg, if available at compile
// -- Michael P. Mendenhall, 2018

#ifndef SVGZOUT_HH
#define SVGZOUT_HH

#include "SVGBuilder.hh"
#include "gzWrapper.hh"
#include "PathUtils.hh"

/// Write D to either .svg or .svgz file; return file extension selected
inline string svgzOut(SVG::SVGDoc& D, const string& outbase, bool gzipIt = true) {
    auto ext = gzipIt && gzOutWrapper::canZip? ".svgz" : ".svg";
    makePath(outbase, true);
    gzOutWrapper o(outbase+ext, gzipIt && gzOutWrapper::canZip);
    D.write(o.f);
    return ext;
}

#endif
