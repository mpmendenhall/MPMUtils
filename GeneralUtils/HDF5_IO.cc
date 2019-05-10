/// \file HDF5_IO.cc
// -- Michael P. Mendenhall, LLNL 2019

#include "HDF5_IO.hh"
#include "PathUtils.hh"
#include <cassert>
#include <climits>

void HDF5_InputFile::openInput(const string& filename) {
    if(infile_id) {
        printf("Closing previous input file.\n");
        H5Fclose(infile_id);
        infile_id = 0;
    }
    if(!filename.size()) return;
    printf("Opening HDF5 input file '%s'\n",filename.c_str());
    infile_id = H5Fopen(filename.c_str(), // file name
                        H5F_ACC_RDONLY,   // access_mode : read only
                        H5P_DEFAULT       // access_ID defaults
    );
}

void HDF5_OutputFile::openOutput(const string& filename) {
    assert(!outfile_id);
    makePath(filename, true);
    printf("Opening HDF5 output file '%s'.\n", filename.c_str());
    outfile_name = filename;
    outfile_id = H5Fcreate(outfile_name.c_str(), // file name
                           H5F_ACC_TRUNC, // access_mode : overwrite old file with new data
                           H5P_DEFAULT,   // create_ID defaults
                           H5P_DEFAULT    // access_ID defaults
    );
}

void HDF5_OutputFile::writeFile() {
    if(!outfile_id) {
        printf("No HDF5 output file opened! Data not saved!\n");
        return;
    }
    printf("Writing events to HDF5 file '%s' and closing...\n", outfile_name.c_str());
    H5Fclose(outfile_id);
    outfile_id = 0;
}

HDF5_OutputFile::~HDF5_OutputFile() {
    assert(!outfile_id); // close file in derived class!
    if(outfile_id) writeFile();
}

string HDF5_InputFile::getAttribute(const string& table, const string& attrname, const string& dflt) {
    assert(infile_id);
    string s = dflt;

    hsize_t dims;
    H5T_class_t type_class;
    size_t type_size;
    herr_t err = H5LTget_attribute_info(infile_id, table.c_str(), attrname.c_str(),  &dims, &type_class, &type_size);
    if(err < 0) return dflt;

    vector<char> sdata(type_size);
    err = H5LTget_attribute_string(infile_id, table.c_str(), attrname.c_str(),  &sdata[0]);
    assert(err >= 0);
    s = string(&sdata[0]);

    return s;
}

double HDF5_InputFile::getAttributeD(const string& table, const string& attrname, double dflt) {
    assert(infile_id);
    double d = dflt;
    herr_t err = H5LTget_attribute_double(infile_id, table.c_str(), attrname.c_str(),  &d);
    if(err < 0) return dflt;
    return d;
}

hsize_t HDF5_InputFile::getTableEntries(const string& table, hsize_t* nfields) {
    if(nfields) *nfields = 0;
    if(!infile_id) return 0;
    hsize_t nf, nrecords;
    herr_t err = H5TBget_table_info(infile_id, table.c_str(), nfields? nfields : &nf, &nrecords);
    assert(err >= 0);
    return nrecords;
}

void HDF5_OutputFile::writeAttribute(const string& table, const string& attrname, double value) {
    assert(outfile_id);
    herr_t err = H5LTset_attribute_double(outfile_id, table.c_str(), attrname.c_str(), &value, 1);
    assert(err >= 0);
}

void HDF5_OutputFile::writeAttribute(const string& table, const string& attrname, const string& value) {
    assert(outfile_id);
    herr_t err = H5LTset_attribute_string(outfile_id, table.c_str(), attrname.c_str(), value.c_str());
    assert(err >= 0);
}
