/// @file DiskIOJobControl.hh MultiJobControl using files on disk for communication

#ifndef DISKIOJOBCONTROL_HH
#define DISKIOJOBCONTROL_HH

#include "MultiJobControl.hh"
#include "DiskBIO.hh"

/*
/// Distribute and collect jobs via filesystem
class DiskIOJobControl: public MultiJobControl {
public:
    /// Destructor
    ~DiskIOJobControl();
    /// initialize
    void init(int argc, char** argv);

    /// blocking data send
    void _send(void* vptr, size_t size) override;
    /// blocking data receive
    void _receive(void* vptr, size_t size) override;

    /// clear output buffer to current destination
    void clearOut() override;
    /// clear input buffer from current source
    void clearIn() override;

    string data_bpath = "./";   ///< base path to data exchange directory

protected:
    map<int, size_t> srcpos;    ///< input buffer position from given source rank
};

/// Distribute and collect jobs via filesystem
class DiskIOJobControl: public MultiJobWorker {
public:
    /// initialize
    void init(int argc, char** argv);

    /// blocking data send
    void _send(void* vptr, size_t size) override;
    /// blocking data receive
    void _receive(void* vptr, size_t size) override;

    /// clear output buffer to current destination
    void clearOut() override;
    /// clear input buffer from current source
    void clearIn() override;


    string data_bpath = "./";   ///< base path to data exchange directory

protected:
    map<int, size_t> srcpos;    ///< input buffer position from given source rank
};
*/

#endif
