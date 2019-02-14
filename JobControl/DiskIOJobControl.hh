/// \file DiskIOJobControl.hh MultiJobControl using files on disk for communication

#ifndef DISKIOJOBCONTROL_HH
#define DISKIOJOBCONTROL_HH

#include "MultiJobControl.hh"
#include <unistd.h>

/// Binary I/O via Unix file descriptors
class FDBinaryIO: public BinaryIO {
public:
    /// Constructor
    FDBinaryIO(int fdIn = -1, int fdOut = -1): fIn(fdIn), fOut(fdOut) { }
    /// Constructor with filenames
    FDBinaryIO(const string& nIn, const string& nOut) { openIn(nIn); openOut(nOut); }
    /// Destructor
    ~FDBinaryIO() { closeIn(); flush(); closeOut(); }

    /// open input file
    void openIn(const string& s);
    /// open output file
    void openOut(const string& s);
    /// close input file
    void closeIn() { if(fIn >= 0) close(fIn); fIn = -1; }
    /// close input file
    void closeOut() { if(fOut >= 0) close(fOut); fOut = -1; }

    /// blocking data send
    void _send(void* vptr, int size) override { if(fOut >= 0 && size != write(fOut, vptr, size)) exit(1); }
    /// blocking data receive
    void _receive(void* vptr, int size) override { if(fIn >= 0 && size != read(fIn, vptr, size)) exit(1); }
    /// flush output
    void flush() override { if(fOut >= 0) fsync(fOut); }

protected:
    int fIn = -1;    ///< input file descriptor
    int fOut = -1;   ///< output file descriptor
};

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

int runSysCmd(const string& cmd);

#endif
