/// @file lindilate.hh Templatized dilation calculation

#include <vector>
#include <limits> // for std::numeric_limits
#include <algorithm> // for std::copy
using std::vector;

/// 1-dimensional dilation by l units
template <typename T>
void lindilate(vector<T>& d, int l) {
    // size of extended region for calculation
    const int w = (int)d.size();
    int exwidth = l*(w/l+1);
    if(!w%l) exwidth--;
    const int o=l/2;

    // calculation intermediate buffers
    vector<T> f(exwidth, -std::numeric_limits<T>::max());
    vector<T> g(exwidth, -std::numeric_limits<T>::max());
    vector<T> h(exwidth, -std::numeric_limits<T>::max());
    std::copy(d.begin(), d.end(), f.begin());

    for(int n=0; n<exwidth; n+=l){
        g[n]=f[n];
        h[exwidth-1-n]=f[exwidth-1-n];
        for(int i=1; i<l; i++){
            if(f[n+i]>g[n+i-1]) g[n+i]=f[n+i];
            else g[n+i]=g[n+i-1];
            if(f[exwidth-1-n-i] > h[exwidth-1-n-i+1]) h[exwidth-1-n-i] = f[exwidth-1-n-i];
            else h[exwidth-1-n-i]=h[exwidth-1-n-i+1];
        }
    }
    for(int i=0; i<w; i++) {
        if(i-o < 0 || (i+l-o-1<exwidth && g[i+l-o-1]>h[i-o])) d[i] = g[i+l-o-1];
        else d[i] = h[i-o];
    }
}
