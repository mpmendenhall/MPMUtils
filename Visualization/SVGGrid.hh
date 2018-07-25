/// \file SVGGrid.hh Helper for making grid of SVG groups

#ifndef SVGGRID_HH
#define SVGGRID_HH

#include "SVGBuilder.hh"
#include "GridLayout.hh"
#include <cassert>

/// place grid of subgroups in element
void makeGrid(const GridLayout<double>& L, vector<shared_ptr<SVG::group>>& els, XMLBuilder& X) {
    // choose grid for group bounding boxes
    vector<BBox<2,double>> vb;
    for(auto& g: els) vb.push_back(g->getBB());
    auto G = L.makeGrid(vb);
    printf("Layed out %i x %i SVG grid (quality %g)\n", G.nrows, G.ncols, G.qual);

    // shift groups to center on grid
    int i=0;
    auto b0 = vb.data();
    for(auto b: G.contents) {
        int r,c;
        G.rowcol(i++,r,c);
        if(!b) continue;
        els[b-b0]->translation[0] += G.ccenter[c] - b->pos(0.5,0);
        els[b-b0]->translation[1] += G.rcenter[r] - b->pos(0.5,1);
    }

    // place groups in grid
    for(auto& g: els) X.addChild(g);
}

#endif
