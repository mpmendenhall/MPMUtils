/// @file DeltaRoot.cc

#include "DeltaRoot.hh"
#include "PathUtils.hh"

#include <TFile.h>
#include <TTree.h>
#include <TKey.h>
#include <TH1.h>
#include <TAxis.h>
#include <TPad.h>

bool DeltaRoot::_compare() {
    TFile f1(fref.c_str(), "READ");
    if(f1.IsZombie()) throw std::runtime_error("Failed opening reference file '"+fref+"'");
    TFile f2(fcomp.c_str(), "READ");
    if(f2.IsZombie()) throw std::runtime_error("Failed opening reference file '"+fcomp+"'");
    return tdcompare(&f1, &f2);
}

//-------------------------------------------
//-------------------------------------------

bool axcompare(const TAxis* a, const TAxis* b) {
    if(!a && !b) return true;
    if(!a || !b) {
        printf("\tInconsistent axis definitions");
        return false;
    }

    auto na = a->GetNbins();
    auto nb = b->GetNbins();
    if(na != nb) {
        printf("\tAxis nbins changed %i -> %i\n", na, nb);
        return false;
    }

    do {
        if(a->GetBinLowEdge(na) != b->GetBinLowEdge(na)) {
            printf("\tAxis range changed\n");
            return false;
        }
    } while(na--);


    return true;
}

//-------------------------------------------

bool hcompare(const TH1& a, const TH1& b) {
    auto na = a.GetNcells();
    auto nb = b.GetNcells();
    if(na != nb) {
        printf("\tbinning has changed %i -> %i\n", na, nb);
        return false;
    }

    auto ea = a.GetEntries();
    auto eb = b.GetEntries();
    if(ea != eb) {
        printf("\tnumber of entries changed %g -> %g\n", ea, eb);
        return false;
    }

    if(!axcompare(a.GetXaxis(), b.GetXaxis())) return false;
    if(!axcompare(a.GetYaxis(), b.GetYaxis())) return false;
    if(!axcompare(a.GetZaxis(), b.GetZaxis())) return false;

    while(--na) {
        if(a.GetBinContent(na) != b.GetBinContent(na)) {
            printf("\tbin contents changed\n");
            return false;
        }
        if(a.GetBinError(na) != b.GetBinError(na)) {
            printf("\tbin errors changed\n");
            return false;
        }
    }

    return true;
}

//-------------------------------------------

bool tcomapare(const TTree& a, const TTree& b) {
    auto na = a.GetEntries();
    auto nb = b.GetEntries();
    if(na != nb) {
        printf("\tnumber of entries changed %g -> %g\n", double(na), double(nb));
        return false;
    }
    return true;
}

//-------------------------------------------
//-------------------------------------------

template<class T>
struct recaster {
    recaster(TObject* o1, TObject* o2):
    a(dynamic_cast<T*>(o1)), b(dynamic_cast<T*>(o2)), good(a && b) { }
    T* a;
    T* b;
    bool good;
};

bool DeltaRoot::tdcompare(TDirectory* d1, TDirectory* d2) const {
    if(!d1 || !d2) throw std::runtime_error("Comparing null directories");

    bool same = true;

    for(auto _k: *d1->GetListOfKeys()) {
        auto k = dynamic_cast<TKey*>(_k);
        if(!k) throw std::logic_error("TKey expected");

        auto name = k->GetName();
        auto k2 = d2->GetKey(name);

        if(!k2) {
            printf("Object '%s' in reference file only\n", name);
            same = false;
            continue;
        }

        if(k->IsFolder() && k2->IsFolder()) {
            printf("Desceding to directory '%s'\n", name);
            DeltaRoot DR;
            DR.outdir = outdir + '/' + name;
            same |= DR.tdcompare(d1->GetDirectory(name), d2->GetDirectory(name));
            continue;
        }

        printf("Comparing '%s'\n", name);
        auto o1 = k->ReadObj();
        auto o2 = k2->ReadObj();

        do {
            auto hh = recaster<TH1>(o1, o2);
            if(hh.good) {
                printf("\tis a TH1\n");
                if(!hcompare(*hh.a, *hh.b)) {
                    same = false;
                    makePath(outdir);
                    hh.a->Draw("Col Z");
                    gPad->Print((outdir + "/" + name + "_old.pdf").c_str());
                    hh.b->Draw("Col Z");
                    gPad->Print((outdir + "/" + name + "_new.pdf").c_str());
                }
                break;
            }

            auto tt = recaster<TTree>(o1, o2);
            if(tt.good) {
                printf("\tis a TTree\n");
                break;
            }

            // might just be comparing the names...
            if(!o1->Compare(o2)) printf("\tAutomatic comparison agrees\n");
            else {
                same = false;
                printf("\tAutomatic comparison differs\n");
            }

        } while(false);


        delete o1;
        delete o2;
    }

    for(auto k: *d2->GetListOfKeys()) {
        auto k2 = d1->GetKey(k->GetName());
        if(!k2) {
            printf("Object '%s' in comparison file only\n", k->GetName());
            same = false;
            continue;
        }
    }

    return same;
}


