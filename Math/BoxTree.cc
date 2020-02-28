/// \file BoxTree.cc

#include "BoxTree.hh"
#include <algorithm>
#include <cassert>
#include <cmath>

BoxTreeNode::iterator BoxTreeNode::begin() {
    iterator it(this);
    it.descend_low();
    return it;
}

BoxTreeNode::const_iterator BoxTreeNode::begin() const {
    const_iterator it(this);
    it.descend_low();
    return it;
}

size_t BoxTreeNode::size() const {
    return 1 + (cLo? cLo->size() : 0) + (cHi? cHi->size() : 0);
}

size_t BoxTreeNode::nLeaves() const {
    if(isLeaf()) return 1;
    return (cLo? cLo->nLeaves() : 0) + (cHi? cHi->nLeaves() : 0);
}

size_t BoxTreeNode::nSplits(int a) const {
    if(isLeaf()) return 0;
    return (a==axis) + (cLo? cLo->nSplits(a) : 0) + (cHi? cHi->nSplits(a) : 0);
}

double BoxTreeNode::bLo(int a) const {
    if(!parent) return -DBL_MAX;
    if(a != parent->axis || isLo()) return parent->bLo(a);
    return parent->split;
}

double BoxTreeNode::bHi(int a) const {
    if(!parent) return DBL_MAX;
    if(a != parent->axis || isHi()) return parent->bHi(a);
    return parent->split;
}

const BoxTreeNode* BoxTreeNode::locate(const double* d) const {
    if(isLeaf()) return this;
    return d[axis] < split? cLo->locate(d) : cHi->locate(d);
}

const BoxTreeNode* BoxTreeNode::locateCenter(const BoxTreeNode& N, map<int,double>& cs) const {
    if(isLeaf()) return this;
    auto it = cs.find(axis);
    double c = 0;
    if(it != cs.end()) c = it->second;
    else c = cs[axis] = N.center(axis);
    return c < split? cLo->locateCenter(N,cs) : cHi->locateCenter(N,cs);
}

void BoxTreeNode::adopt() {
    if(cLo) cLo->parent = this;
    if(cHi) cHi->parent = this;
}

BoxTreeNode* BoxTreeNode::clone() const {
    auto B2 = new BoxTreeNode();
    B2->axis = axis;
    B2->split = split;
    if(cLo) B2->cLo = cLo->clone();
    if(cHi) B2->cHi = cHi->clone();
    B2->adopt();
    return B2;
}

bool BoxTreeNode::_split(int a, double s, BoxTreeNode*& newLo, BoxTreeNode*& newHi) {
    if(isLeaf()) {
        newLo = this;
        newHi = new BoxTreeNode();
        return true;
    }

    if(a==axis) {
        if(s < split) {
            cLo->_split(a, s, newLo, cLo);
            newHi = this;
        } else if(s==split) {
            newLo = cLo;
            newHi = cHi;
            return false;
        } else {
            cHi->_split(a, s, cHi, newHi);
            newLo = this;
        }
    } else {
        newLo = this;
        newHi = new BoxTreeNode();
        newHi->axis = axis;
        newHi->split = split;
        if(cLo) cLo->_split(a, s, cLo, newHi->cLo);
        if(cHi) cHi->_split(a, s, cHi, newHi->cHi);
        newHi->adopt();
    }
    adopt();
    return true;
}

BoxTreeNode* BoxTreeNode::splitNode(int a, double s) {
    if(cHi && a==axis && s==split) return this; // already split here!
    auto pOld = parent;
    auto loOld = isLo();

    BoxTreeNode* newLo;
    BoxTreeNode* newHi;
    auto N = _split(a, s, newLo, newHi)? new BoxTreeNode() : this;
    N->axis = a;
    N->split = s;
    N->cLo = newLo;
    N->cHi = newHi;
    N->adopt();

    if(pOld) {
        (loOld? pOld->cLo : pOld->cHi) = N;
        N->parent = pOld;
    }
    return N;
}

BoxTreeNode* BoxTreeNode::splitNode(const BoxTreeNode* N) {
    if(N->isLeaf()) return this;
    auto B = splitNode(N->axis, N->split);
    B->cLo->splitNode(N->cLo);
    B->cHi->splitNode(N->cHi);
    return B;
}

BoxTreeNode* BoxTreeNode::bound(int a, double s0, double s1) {
    if(s0 > s1) std::swap(s0,s1);
    auto lo = bLo(a);
    auto hi = bHi(a);
    if(s0 > hi || s1 < lo) return nullptr; // impossible request!
    auto B = this;
    if(lo < s0) B = B->splitNode(a,s0)->cHi;
    if(s1 < hi) B = B->splitNode(a,s1)->cLo;
    return B;
}

void BoxTreeNode::findNodes(vector<const BoxTreeNode*>& v, std::function<bool(const BoxTreeNode&)> f) const {
    if(f(*this)) v.push_back(this);
    else {
        if(cLo) cLo->findNodes(v,f);
        if(cHi) cHi->findNodes(v,f);
    }
}

void BoxTreeNode::findLeafNodes(vector<const BoxTreeNode*>& v, std::function<bool(const BoxTreeNode&)> f) const {
    if(!f(*this)) return;
    if(isLeaf()) v.push_back(this);
    else {
        if(cLo) cLo->findLeafNodes(v,f);
        if(cHi) cHi->findLeafNodes(v,f);
    }
}

BoxTreeNode* BoxTreeNode::projectOut(int a) {
    if(isLeaf()) return this;

    cLo = cLo->projectOut(a);
    cHi = cHi->projectOut(a);

    if(a != axis) return this;

    auto N = cLo->splitNode(cHi);
    cLo = nullptr;

    N->parent = parent;
    if(parent) (isLo()? parent->cLo : parent->cHi) = N;
    delete this;

    return N;
}

//////////////////////

void KDBuilder::initData(const vector<float*>& ps) {
    printf("Initializing KDBuilder for %zu %i-dimensional datapoints\t", ps.size(), N_DIM); fflush(stdout);
    for(int a=0; a<N_DIM; a++) {
        psorted[a].assign(ps.begin(), ps.end());
        std::sort(psorted[a].begin(), psorted[a].end(), [=](const float* f1, const float* f2) { return f1[a] < f2[a]; });
        printf("*"); fflush(stdout);
    }
    printf("    Done.\n");
}

BoxTreeNode* KDBuilder::boundData(double xr, BoxTreeNode* T) const {
    if(!T) T = new BoxTreeNode();
    for(int a=0; a<N_DIM; a++) {
        auto mn = psorted[a][0][a];
        auto mx = psorted[a].back()[a];
        auto dr = mx-mn;
        printf("Bounding data[%i] range %g -- %g\n", a, mn, mx);
        T = T->bound(a, mn-xr*dr, mx+xr*dr);
    }
    return T;
}

unsigned int choose_divider(float** f0, float** f1, int ax, double& rbest) {
    const auto N = f1-f0;
    auto Nc = N/2;

    auto x0 = (*f0)[ax];
    auto xm = (*(f0 + Nc))[ax];
    auto x2 = (*(f1-1))[ax];

    bool dx = true;
    Nc++;
    rbest = 0;
    while(Nc > 0 && Nc < N-1) {
        auto xn = (*(f0 + Nc))[ax];
        auto x1 = 0.5*(xn+xm);

        auto Ncc = N-1-Nc;
        auto v0 = x1-x0;
        auto v1 = x2-x1;
        auto r = (Nc*Nc*v1*v1 + Ncc*Ncc*v0*v0 - 2*Nc*Ncc*v0*v1)/(Nc*v1*v1 + Ncc*v0*v0);
        if(r <= rbest) {
            dx = !dx;     // switch search direction
            if(dx) break; // ... only once
            Nc--;         // go back to previous spot
        } else {
            xm = xn;
            rbest = r;
        }

        if(dx) Nc++;
        else Nc--;
    }

    return Nc;
}

void KDBuilder::partition(size_t N0, size_t N1, size_t Nc, int ax) {

    // set aside datapoints for enumeration
    vector<float> p0(N1-N0);
    for(auto i = N0; i < N1; i++) {
        p0[i-N0] = psorted[ax][i][ax];
        psorted[ax][i][ax] = i;
    }

    for(int a=0; a<N_DIM; a++) {
        std::stable_partition(psorted[a].begin()+N0, psorted[a].begin()+N1,
                              [=](const float* f) { return f[ax] < Nc; });
    }

    // restore enumerated datapoints
    for(auto i = N0; i < N1; i++) psorted[ax][i][ax] = p0[i-N0];
}

BoxTreeNode* KDBuilder::buildKD(size_t N0, size_t N1, map<const BoxTreeNode*,double>& leafcounts, BoxTreeNode* T) {
    assert(N1 > N0);
    assert(N1 <= psorted[0].size());
    if(!T) T = new BoxTreeNode();

    if(!T->isLeaf()) {
        auto ax = T->getAxis();
        auto afind = [=](const float* f1, const float f2) { return f1[ax] < f2; };
        auto Nc = std::lower_bound(psorted[ax].begin()+N0, psorted[ax].begin()+N1, T->getSplit(), afind)-psorted[ax].begin();
        partition(N0,N1,Nc,ax);
        buildKD(N0, Nc, leafcounts, T->getLo());
        buildKD(Nc, N1, leafcounts, T->getHi());
        return T;
    }

    if(N1-N0 < min_divide_points) {
        if(closeBounds) {
            for(int a=0; a<N_DIM; a++) {
                bool bl = T->isBoundedLo(a);
                bool bh = T->isBoundedHi(a);
                if(bl && bh) continue;
                auto mn = psorted[a][N0][a];
                auto mx = psorted[a][N1-1][a];
                auto dr = mx-mn;
                if(!bl) T = T->splitNode(a,mn-0.1*dr)->getHi();
                if(!bh) T = T->splitNode(a,mx+0.1*dr)->getLo();
            }
        }
        leafcounts[T] = N1-N0;
        return T;
    }

    // determine best axis to split
    //double spanmin = DBL_MAX;
    //double spanmax = 0;
    //int along = 0;
    double rmax = 0;
    int amin = 0;
    size_t Nc = (N0+N1)/2;
    for(int a=0; a<N_DIM; a++) {
        //double aspan = T->span(a);
        //spanmin = std::min(spanmin, aspan);
        //spanmax = std::max(spanmax, aspan);
        //if(spanmax == aspan) along = a;

        double r;
        auto NcNew = Nc;

        if(smartDivide) NcNew = N0 + choose_divider(&psorted[a][N0], &psorted[a][N1], a, r);
        else {
            auto x0 = psorted[a][N0][a];
            auto x1 = psorted[a][Nc][a];
            auto x2 = psorted[a][N1-1][a];
            r = fabs(x1-0.5*(x0+x2))/(x2-x0);
        }

        if(r>rmax) { rmax = r; amin = a; Nc=NcNew; }
    }
    //double rlim = 1./sqrt(N);
    //if(rmax < 2*rlim) amin = along; // consider splitting in long direction if near uniform
    //if(rmax < rlim && 4*spanmin > spanmax) return T;

    // recommend split position
    float xc = 0.5*(psorted[amin][Nc][amin]+psorted[amin][Nc-1][amin]);

    // round to nearest "reasonable" value
    if(snapgrid) {
        auto x0 = psorted[amin][N0][amin];
        auto x1 = psorted[amin][N1-1][amin];
        double u = pow(2., floor(log(x1-x0)/log(2)-snapgrid));
        xc = round((xc-0.5)/u)*u + 0.5;
        if(xc <= x0) xc += u;
        if(xc >= x1) xc -= u;
        auto afind = [=](const float* f1, const float f2) { return f1[amin] < f2; };
        Nc = std::lower_bound(psorted[amin].begin()+N0, psorted[amin].begin()+N1, xc, afind)-psorted[amin].begin();
    }
    if(Nc <= N0 || Nc >= N1-1) return T;

    // perform split and recurse
    T = T->splitNode(amin, xc);
    partition(N0, N1, Nc, amin);
    buildKD(N0, Nc, leafcounts, T->getLo());
    buildKD(Nc, N1, leafcounts, T->getHi());
    return T;
}

BoxTreeNode* KDBuilder::buildKD(map<const BoxTreeNode*,double>& leafcounts, BoxTreeNode* T) {
    auto N = psorted[0].size();
    printf("Constructing KD Tree to partition %zu points...\n", N);
    T = buildKD(0, N, leafcounts, T);
    printf("    built with %zu nodes, %zu leaves, depth %u\n", T->size(), T->nLeaves(), T->maxdepth());
    return T;
}
