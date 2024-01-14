/// @file BurstFill.hh Correlated burst histogram fill
// -- Michael P. Mendenhall, LLNL 2020

#ifndef BURSTFILL_HH
#define BURSTFILL_HH

#include "MultiFill.hh"
#include "ClusteredWindow.hh"
#include "OrderingQueue.hh"
#include "OrderedData.hh"

/// Covariance matrix paired to histogram for correlated-bin fills
class BurstFill: public MultiFill, public Clusterer<OrderedData<int>> {
public:
    /// Constructor, corresponding to histogram
    BurstFill(const string& _name, TH1& H): MultiFill(_name, H), ClusterBuilder(500e3), OQ(this, 2e9) { }
    /// Constructor, loaded from file
    BurstFill(const string& _name, TDirectory& d, TH1& H): MultiFill(_name, d, H), ClusterBuilder(500e3), OQ(this, 2e9) { }

    /// fill (1D) with event time
    void tFill(double t, double x) { OQ.push({t, h->FindBin(x)}); }
    /// fill (2D) with event time
    void tFill(double t, double x, double y) { OQ.push({t, h->FindBin(x,y)}); }

    /// handle datastream signals
    void signal(datastream_signal_t sig) override {
        if(signore) return;
        signore = true;
        OQ.signal(sig);
        signore = false;
        Clusterer<OrderedData<int>>::signal(sig);
    }

    /// end-of-data operations
    void endFill() override { signal(DATASTREAM_END); MultiFill::endFill(); }

protected:
    typedef OrderedData<int> ordbin_t;
    typedef Cluster<ordbin_t> cluster_t;

    /// pre-ordering buffer
    OrderingQueue<const ordbin_t> OQ;

    /// fill from cluster
    bool checkCluster(cluster_t& o) override {
        fillBins(vector<ordbin_t::val_extractor>(o.begin(), o.end()));
        return o.size();
    }

    bool signore = false;   ///< lock to ignore signal loop from OQ
};

#endif
