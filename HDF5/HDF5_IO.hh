/// \file HDF5_IO.hh HDF5 file I/O
// -- Michael P. Mendenhall, LLNL 2019

#ifndef HDF5_IO_HH
#define HDF5_IO_HH

#include "hdf5.h"
#include "hdf5_hl.h"
#include <string>
using std::string;
#include <stdexcept>
#include "DataSource.hh"

/// base class for HDF5 file input
class HDF5_InputFile: virtual public _FileSource {
public:
    /// Destructor
    virtual ~HDF5_InputFile() { if(infile_id) H5Fclose(infile_id); }
    /// Open named input file
    void openInput(const string& filename) override;

    /// get number of records in input table
    hsize_t getTableEntries(const string& table, hsize_t* nfields = nullptr);
    /// check whether named object has named attribute
    bool doesAttrExist(const string& objname, const string& attrname) const;
    /// read double-valued attribute
    double getAttributeD(const string& table, const string& attrname, double dflt) const;
    /// read string-valued attribute
    string getAttribute(const string& table, const string& attrname, const string& dflt) const;

    hid_t infile_id = 0;    ///< input HDF5 file ID
};

/// base class for HDF5 file output
class HDF5_OutputFile {
public:
    /// Destructor: please close file before destructing
    virtual ~HDF5_OutputFile();
    /// Open named output file; connect table writers in subclass!
    virtual void openOutput(const string& filename);
    /// Finalize/close file output; close writers in subclass first!
    virtual void writeFile();
    /// Whether output file is open
    bool outIsOpen() const { return outfile_id; }

    /// write double-valued attribute
    void writeAttribute(const string& table, const string& attrname, double value);
    /// write string-valued attrivute
    void writeAttribute(const string& table, const string& attrname, const string& value);

    string outfile_name;        ///< output filename
    hid_t outfile_id = 0;       ///< output HDF5 file ID
};

#endif
