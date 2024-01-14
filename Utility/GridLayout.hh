/// @file GridLayout.hh Helper for laying out grids of bounding boxes

#ifndef GRIDLAYOUT_HH
#define GRIDLAYOUT_HH

#include "BBox.hh"
#include <cassert>
#include <float.h>
#include <algorithm>

/// get sort ordering indexes
template <typename T>
vector<size_t> sort_indices(const vector<T> &v) {
    vector<size_t> idx(v.size());
    std::iota(idx.begin(), idx.end(), 0);
    std::sort(idx.begin(), idx.end(),[&v](size_t i1, size_t i2) {return v[i1] < v[i2];});
    return idx;
}

vector<size_t> sort_order(const vector<size_t>& idx) {
    vector<size_t> so(idx.size());
    size_t j=0;
    for(auto i: idx) so[i] = j++;
    return so;
}

/// Grid layout optimization template class
template<typename T>
class GridLayout {
public:

    /// convenience typedef for 2D bounding box
    typedef BBox<2,T> BBx;
    /// convenience typedef for const pointer to bounding box
    typedef const BBx* pBBx;

    /// description of a layout
    struct Gridspec {
        /// Constructor with rows, columns
        Gridspec(int nr=0, int nc=0): nrows(nr), ncols(nc) { }

        int nrows;  ///< number of rows
        int ncols;  ///< number of columns
        vector<double> widths;  ///< column widths
        double width;           ///< total width
        vector<double> heights; ///< row heights
        double height;          ///< total height
        vector<double> ccenter; ///< column centers
        vector<double> rcenter; ///< row centers
        vector<pBBx> contents;  ///< contents of each table cell; nullptr for empty
        double afill;           ///< filled area
        double qual = 0;        ///< optimization quality

        /// total size
        int size() const { return nrows*ncols; }
        /// row, column from index
        void rowcol(int i, int& r, int& c) const { r = i/ncols; c = i%ncols; }
        /// index from row, column
        int index(int r, int c) { return r*ncols + c; }

        /// calculate dimensions from max in row/column
        void calcDims() {
            assert((int)contents.size() == nrows*ncols);

            widths.resize(ncols);
            std::fill(widths.begin(), widths.end(), 0);
            heights.resize(nrows);
            std::fill(heights.begin(), heights.end(), 0);

            int i=0;
            afill = 0;
            for(auto b: contents) {
                int r,c;
                rowcol(i++,r,c);
                if(!b) continue;
                afill += b->dl(0)*b->dl(1);
                widths[c] = std::max(widths[c], b->dl(0));
                heights[r] = std::max(heights[r], b->dl(1));
            }
            width = std::accumulate(widths.begin(), widths.end(), 0.);
            height = std::accumulate(heights.begin(), heights.end(), 0.);
        }

        /// calculate column centers
        void calcCenters() {
            ccenter.resize(ncols);
            rcenter.resize(nrows);
            std::partial_sum(widths.begin(), widths.end(), ccenter.begin());
            std::partial_sum(heights.begin(), heights.end(), rcenter.begin());
            auto itw = widths.begin();
            for(auto& x: ccenter) x -= (*(itw++))/2.;
            auto ith = heights.begin();
            for(auto& x: rcenter) x -= (*(ith++))/2.;
        }

        /// display info
        void display() const {
            printf("%i x %i grid\nrows:", nrows, ncols);
            for(auto x: heights) printf("\t%g",x);
            printf("\n\t");
            for(auto x: rcenter) printf("\t%g",x);
            printf("\ncols:");
            for(auto x: widths) printf("\t%g",x);
            printf("\n\t");
            for(auto x: ccenter) printf("\t%g",x);
            printf("\n");
            for(int r=0; r<nrows; r++) {
                for(int c=0; c<ncols; c++) {
                    int i = r*ncols+c;
                    if(!contents[i]) printf("\t(----------)");
                    else printf("\t%g x %g", contents[i]->dl(0), contents[i]->dl(1));
                }
                printf("\n");
            }
        }

        /// swap two columns
        void swap_cols(size_t i, size_t j) {
            if(i==j) return;
            for(int r=0; r<nrows; r++) std::swap(contents[index(r,i)], contents[index(r,j)]);
            std::swap(widths[i],widths[j]);
        }

        /// swap two rows
        void swap_rows(size_t i, size_t j) {
            if(i==j) return;
            for(int c=0; c<ncols; c++) std::swap(contents[index(i,c)], contents[index(j,c)]);
            std::swap(heights[i],heights[j]);
        }

        /// permute into standardized order
        void canonical_order() {
            auto ri = sort_indices(heights);
            ri = sort_order(ri);
            for(int r=0; r<nrows; r++) {
                if((int)ri[r] == r) continue;
                swap_rows(ri[r], r);
                std::swap(ri[r], ri[ri[r]]);
                r=-1;
            }

            auto ci = sort_indices(widths);
            ci = sort_order(ci);
            for(int c=0; c<ncols; c++) {
                if((int)ci[c] == c) continue;
                swap_cols(ci[c], c);
                std::swap(ci[c], ci[ci[c]]);
                c=-1;
            }
        }
    };

    static bool rsort_height(pBBx a, pBBx b) { return b->dl(1) < a->dl(1); }

    /// fill grid contents
    void fillGrid(Gridspec& G, const vector<BBx>& bxs) const {
        assert(G.nrows*G.ncols >= (int)bxs.size());
        G.contents.clear();
        for(auto& b: bxs) G.contents.push_back(&b);
        if(reorder) std::sort(G.contents.begin(), G.contents.end(), &rsort_height);
        G.contents.resize(G.nrows*G.ncols, nullptr); // remainder empty cells
    }

    /// quality metric for arrangement
    double quality(const Gridspec& G) const {
        double q = G.afill/(G.width*G.height);
        if(G.width < w2h*G.height) q *= G.width/(G.height*w2h);
        else q *= G.height*w2h/G.width;
        return q;
    }

    /// try pair-swap permutations on a grid; return best quality
    double trypermutes(Gridspec& G) const {
        G.calcDims();
        double qbest = quality(G);
        int ri,rj,ci,cj;
        for(int i=0; i<G.size(); i++) {
            G.rowcol(i,ri,ci);

            for(int j=i+1; j<G.size(); j++) {

                auto& bi = G.contents[i];
                auto& bj = G.contents[j];
                if(!bi && !bj) continue;

                G.rowcol(j,rj,cj);
                std::swap(bi,bj);
                G.calcDims();
                double q = quality(G);

                if(q > qbest) {
                    qbest = q;
                    i = -1;
                    break;
                }

                if(q==qbest) { // check for improved ordering
                    // watch out, they've already been swapped!
                    auto wi = bi? bi->dl(0) : 0;
                    auto wj = bj? bj->dl(0) : 0;
                    auto hi = bi? bi->dl(1) : 0;
                    auto hj = bj? bj->dl(1) : 0;
                    if( ((wi>wj) != (G.widths[ci]>G.widths[cj])) && ((hi>hj) != (G.heights[ri]>G.heights[rj])) ) {
                        i=-1;
                        break;
                        //continue;
                    }
                }

                std::swap(bi, bj); // restore initial order
            }
        }
        G.calcDims();
        return qbest;
    }

    /// optimize grid layout for BBox collection
    Gridspec makeGrid(const vector<BBx>& bxs) const {
        Gridspec bestGrid;
        double qbest = 0;
        for(size_t nr=1; nr<=bxs.size(); nr++) {
            Gridspec G(nr, bxs.size()/nr + (bxs.size()%nr != 0));
            fillGrid(G,bxs);
            G.calcDims();
            double q = reorder? trypermutes(G) : quality(G);
            if(q > qbest) {
                qbest = q;
                bestGrid = G;
            }
        }
        if(reorder) bestGrid.canonical_order();
        bestGrid.calcCenters();
        bestGrid.qual = qbest;
        return bestGrid;
    }

    double w2h = 1;         ///< target aspect ratio, width/height
    bool reorder = true;    ///< whether we are allowed to reorder contents
};

#endif
