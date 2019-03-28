/// \file DiskIOJobControl.hh MultiJobControl using files on disk for communication

#ifndef DISKIOJOBCONTROL_HH
#define DISKIOJOBCONTROL_HH

#include "MultiJobControl.hh"
#include "DiskBIO.hh"

/// Distribute and collect jobs via filesystem
class DiskIOJobControl: public MultiJobControl {
public:
    /// initialize
    void init(int argc, char** argv) override;
    /// end-of-run completion
    void finish() override;
    /// blocking data send
    void _send(void* vptr, int size) override;
    /// blocking data receive
    void _receive(void* vptr, int size) override;
    /// clear output buffer to current destination
    void clearOut() override;
    /// clear input buffer from current source
    void clearIn() override;


    string data_bpath = "./";   ///< base path to data exchange directory

protected:
    map<int, size_t> srcpos;    ///< input buffer position from given source rank
};

#endif
