/// \file DiskBIO.hh Binary I/O to files

#ifndef DISKBIO_HH
#define DISKBIO_HH

#include "BinaryIO.hh"
#include <iostream>
#include <fstream>
#include <unistd.h>

/// Binary I/O via iostream objects
class IOStreamBIO: virtual public BinaryIO {
public:
    /// Constructor
    IOStreamBIO(std::istream* i, std::ostream* o): fIn(i), fOut(o) { }

    /// blocking data send
    void _send(void* vptr, int size) override { if(fOut) fOut->write((char*)vptr, size); }
    /// blocking data receive
    void _receive(void* vptr, int size) override { if(fIn) fIn->read((char*)vptr, size); }

    std::istream* fIn = nullptr;    ///< input stream
    std::ostream* fOut = nullptr;   ///< output stream
};

/// Binary I/O via Unix file descriptors
class FDBinaryIO: virtual public BinaryIO {
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

int runSysCmd(const string& cmd);

#endif
