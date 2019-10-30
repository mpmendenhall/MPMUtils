/// \file HDF5_IO.hh HDF5 file I/O
// -- Michael P. Mendenhall, LLNL 2019

#ifndef HDF5_IO_HH
#define HDF5_IO_HH

#include "HDF5_StructInfo.hh"
#include "HDF5_Table_Cache.hh"

/// base class for HDF5 file input
class HDF5_InputFile {
public:
    /// Constructor
    HDF5_InputFile(const string& fname = "") { openInput(fname); }
    /// Destructor
    virtual ~HDF5_InputFile() { if(infile_id) H5Fclose(infile_id); }
    /// Open named input file; connect table readers in subclass!
    virtual void openInput(const string& filename);

    /// get number of records in input table
    hsize_t getTableEntries(const string& table, hsize_t* nfields = nullptr);
    /// read double-valued attribute
    double getAttributeD(const string& table, const string& attrname, double dflt = 0);
    /// read string-valued attribute
    string getAttribute(const string& table, const string& attrname, const string& dflt = "");

    hid_t infile_id = 0;    ///< input HDF5 file ID
};

/// HDF5_InputFile with specific table
template<class T>
class HDF5_TableInput: public HDF5_InputFile, public HDF5_Table_Cache<T> {
public:
    /// Constructor
    HDF5_TableInput(const string& tname = "", int v = 0, int nch = 1024): HDF5_InputFile(""),
    HDF5_Table_Cache<T>(HDF5_table_setup<T>(tname, v), nch) { }

    /// Open named input file
    void openInput(const string& filename) override {
        HDF5_InputFile::openInput(filename);
        this->setFile(infile_id);
    }

    /// get number of records in input table
    hsize_t getEntries() { return getTableEntries(this->Tspec.table_name); }
    /// read double-valued attribute
    double getAttributeD(const string& attrname, double dflt = 0) { return HDF5_InputFile::getAttributeD(this->Tspec.table_name, attrname, dflt); }
    /// read string-valued attribute
    string getAttribute(const string& attrname, const string& dflt = "") { return HDF5_InputFile::getAttribute(this->Tspec.table_name, attrname, dflt); }
};


/// base class for HDF5 file output
class HDF5_OutputFile {
public:
    /// Constructor
    HDF5_OutputFile(const string& fname = "") { if(fname.size()) openOutput(fname); }
    /// Destructor
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

/// HDF5_OutputFile with specific table
template<class T>
class HDF5_TableOutput: public HDF5_OutputFile, public HDF5_Table_Writer<T> {
public:
    /// Constructor
    HDF5_TableOutput(const string& tname = "", int v = 0, int nch = 1024): HDF5_OutputFile(""),
    HDF5_Table_Writer<T>(HDF5_table_setup<T>(tname, v),nch) { }

    /// Destructor
    ~HDF5_TableOutput() { if(outfile_id) writeFile(); }

    /// Open named output file
    void openOutput(const string& filename) override {
        HDF5_OutputFile::openOutput(filename);
        this->setFile(outfile_id);
        if(outfile_id) this->initTable();
    }
    /// Finalize/close file output
    void writeFile() override {
        this->setFile(0);
        HDF5_OutputFile::writeFile();
    }
};

#endif
