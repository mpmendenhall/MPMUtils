/// @file PointCloudHistogram.cc

#include "PointCloudHistogram.hh"

void KDTreeSet::finalize() {
    if(T) throw std::logic_error("KDTreeSet T already constructed");

    printf("Building %zu-point, %zu-dimensional kd-tree...\n", nPts(), nDim());

    T = new TKDTree<int,float>(nPts(), nDim(), 1);
    int i = 0;
    for(auto& v: *this) {
        if(v.size() < nPts()) throw std::logic_error("mismatched data array sizes");
        T->SetData(i++, v.data());
    }
    T->Build();
}

void KDTreeSet::remove_points(const vector<size_t>& vidx) {
    clearTree();
    auto npts = nPts();
    size_t i0 = 0;
    auto e = vidx.begin();
    for(size_t i = 0; i < npts; ++i) {
        if(e != vidx.end() && i == *e) { ++e; continue; }
        for(auto& v: *this) v[i0] = v[i];
        ++i0;
    }
    for(auto& v: *this) v.resize(i0);
}

float KDTreeSet::project(size_t i, const float* v) const {
    float s = 0;
    for(unsigned int j=0; j<nDim(); ++j) s += (*this)[j][i] * v[j];
    return s;
}

//---------------------------

void PointCloudHistogram::Fill(const float* x, float v) {
    if(!myTree->T) throw std::logic_error("Binning KDTree undefined");
    int idx = -1;
    float dist = -1;
    myTree->T->FindNearestNeighbors(x, 1, &idx, &dist);
    if(idx < 0) {
        printf("Failed to locate point {");
        for(size_t i = 0; i < myTree->nDim(); ++i) printf("%g ", x[i]);
        printf(" }\n");
        return;
    }
    at(idx) += v;
}

void PointCloudHistogram::project(const float* v, TGraph& g) const {
    size_t b = 0;
    for(auto x: *this) {
        auto s = myTree->project(b,v);
        g.SetPoint(b++, s, x);
    }
    g.Sort();
}

void PointCloudHistogram::project(const float* v, TH1& h) const {
    size_t b = 0;
    for(auto x: *this) {
        auto s = myTree->project(b++, v);
        h.Fill(s, x);
    }
}
