/// @file DiskBIO.hh Binary I/O to files

#ifndef DISKBIO_HH
#define DISKBIO_HH

#include "BinaryIO.hh"
#include <iostream>
#include <fstream>
#include <unistd.h>

/// Binary write to iostream objects
class IOStreamBWrite: virtual public BinaryWriter {
public:
    /// Constructor
    explicit IOStreamBWrite(std::ostream& o): fOut(o) { }

protected:
    std::ostream& fOut;   ///< output stream

    /// blocking data send
    void _send(const void* vptr, size_t size) override { fOut.write(static_cast<const char*>(vptr), size); }
};

/// Binary read from iostream objects
class IOStreamBRead: virtual public BinaryReader {
public:
    /// Constructor
    explicit IOStreamBRead(std::istream& i): fIn(i) { }

    /// blocking data receive
    void read(void* vptr, size_t size) override { if(read_upto(vptr,size) != size) throw std::runtime_error("File out of data"); }
    /// non-blocking opportunistic read of all available to size
    size_t read_upto(void* vptr, size_t s) override { fIn.read(static_cast<char*>(vptr), s); return fIn.gcount(); }

    /// skip over n bytes
    void ignore(size_t n) override { fIn.ignore(n); }

protected:
    std::istream& fIn;  ///< input stream
};


/// Binary write via Unix file descriptors
class FDBinaryWriter: virtual public BinaryWriter {
public:
    /// Constructor
    explicit FDBinaryWriter(int fdOut = -1): fOut(fdOut) { }
    /// Constructor with filenames
    explicit FDBinaryWriter(const string& nOut, bool append = false) { openOut(nOut, append); }
    /// Destructor
    ~FDBinaryWriter() { FDBinaryWriter::flush(); closeOut(); }

    /// open output file
    void openOut(const string& s, bool append = false);
    /// close input file
    void closeOut();
    /// check if output open
    bool outIsOpen() const { return fOut != -1; }

protected:
    /// blocking data send
    void _send(const void* vptr, size_t size) override;
    /// flush output
    void flush() override { if(fOut >= 0 && fsync(fOut)) throw std::runtime_error("failed to fsync output file"); }

    int fOut = -1;   ///< output file descriptor
};


/// Binary read via Unix file descriptors
class FDBinaryReader: virtual public BinaryReader {
public:
    /// Constructor
    explicit FDBinaryReader(int fdIn = -1): fIn(fdIn) { }
    /// Constructor with filenames
    explicit FDBinaryReader(const string& nIn) { openIn(nIn); }
    /// Destructor
    ~FDBinaryReader() { closeIn(); }

    /// open input file
    void openIn(const string& s);
    /// close input file
    void closeIn() { if(fIn >= 0) close(fIn); fIn = -1; }
    /// check if input open
    bool inIsOpen() const { return fIn != -1; }

    /// blocking data receive
    void read(void* vptr, size_t size) override {
        if(fIn < 0) throw std::runtime_error("No input file open!");
        if(size != (size_t)::read(fIn, vptr, size)) throw std::runtime_error("Requested read failed!");
    }
    /// non-blocking opportunistic read of all available to size
    size_t read_upto(void* vptr, size_t s) override { return ::read(fIn, vptr, s); }

protected:
    int fIn = -1;    ///< input file descriptor
};

#endif
