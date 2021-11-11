/// \file MCTAL_toROOT.cc

#include "MCTAL_toROOT.hh"

TH1D* tallyH1(const string& name, const string& title,
              const MCTAL_Tally& t, tallyax_id_t a, size_t i0) {
    if(a == AXIS_END) a = t.axes.at(0);

    auto& Ax = t.axis(a);
    auto vbins = Ax.toVec();

    TH1D* h = new TH1D(name.c_str(), title.c_str(),
                       vbins.size() - 1, vbins.data());
    h->GetXaxis()->SetTitle(Ax.title.c_str());
    for(size_t n = 0; n < vbins.size(); ++n) {
        auto& ve = t.at(i0 + n * Ax.stride);
        h->SetBinContent(n, ve.val);
        h->SetBinError(n, ve.rel_err * ve.val);
    }

    return h;
}

TH2D* tallyH2(const string& name, const string& title,
              const MCTAL_Tally& t, tallyax_id_t a1,
              tallyax_id_t a2, size_t i0) {

    if(a1 == AXIS_END) a1 = t.axes.at(0);
    if(a2 == AXIS_END) a2 = t.axes.at(1);

    auto& Ax1 = t.axis(a1);
    auto vbins1 = Ax1.toVec();

    auto& Ax2 = t.axis(a2);
    auto vbins2 = Ax2.toVec();

    TH2D* h = new TH2D(name.c_str(), title.c_str(),
                       vbins1.size() - 1, vbins1.data(),
                       vbins2.size() - 1, vbins2.data());
    h->GetXaxis()->SetTitle(Ax1.title.c_str());
    h->GetYaxis()->SetTitle(Ax2.title.c_str());
    for(size_t n = 0; n < vbins1.size(); ++n) {
        for(size_t m = 0; m < vbins2.size(); ++m) {
            auto& ve = t.at(i0 + n * Ax1.stride + m * Ax2.stride);
            h->SetBinContent(n, m, ve.val);
            h->SetBinError(n, m, ve.rel_err * ve.val);
        }
    }

    return h;
}

