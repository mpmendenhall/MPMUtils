/// @file HDF5_IO.cc
// -- Michael P. Mendenhall, LLNL 2019

#include "HDF5_IO.hh"
#include "PathUtils.hh" // for makePath
#include <climits>

void HDF5_InputFile::openInput(const string& filename) {
    _FileSource::openInput(filename);
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
    if(outfile_id) throw std::logic_error("Output already open");
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
    printf("Writing data to HDF5 file '%s' and closing...\n", outfile_name.c_str());
    H5Fclose(outfile_id);
    outfile_id = 0;
}

bool HDF5_InputFile::doesAttrExist(const string& objname, const string& attrname) const {
    if(!infile_id) throw std::runtime_error("Cannot read attribute without file");
    auto res = H5Aexists_by_name(infile_id, objname.c_str(), attrname.c_str(), H5P_DEFAULT);
    if(res < 0) throw std::runtime_error("H5Aexists_by_name failed");
    return res;
}

string HDF5_InputFile::getAttribute(const string& table, const string& attrname, const string& dflt) const {
    if(!doesAttrExist(table, attrname)) return dflt;

    hsize_t dims;
    H5T_class_t type_class;
    size_t type_size;
    if(H5LTget_attribute_info(infile_id, table.c_str(), attrname.c_str(),  &dims, &type_class, &type_size) < 0)
        throw std::runtime_error("H5LTget_attribute_info error");

    vector<char> sdata(type_size);
    if(H5LTget_attribute_string(infile_id, table.c_str(), attrname.c_str(),  &sdata[0]) < 0)
        throw std::runtime_error("H5LTget_attribute_string error");
    return string(&sdata[0]);
}

double HDF5_InputFile::getAttributeD(const string& table, const string& attrname, double dflt) const {
    if(!doesAttrExist(table, attrname)) return dflt;

    double d = dflt;
    if(H5LTget_attribute_double(infile_id, table.c_str(), attrname.c_str(),  &d) < 0)
        throw std::runtime_error("H5LTget_attribute_double error");
    return d;
}

hsize_t HDF5_InputFile::getTableEntries(const string& table, hsize_t* nfields) {
    if(nfields) *nfields = 0;
    if(!infile_id) return 0;
    hsize_t nf, nrecords;
    herr_t err = H5TBget_table_info(infile_id, table.c_str(), nfields? nfields : &nf, &nrecords);
    if(err < 0) throw std::runtime_error("H5TBget_table_info error");
    return nrecords;
}

//------------------------

HDF5_OutputFile::~HDF5_OutputFile() {
    if(outfile_id) {
        printf("Need to HDF5_OutputFile::writeFile() before deleting!\n");
        abort();
    }
}

void HDF5_OutputFile::writeAttribute(const string& table, const string& attrname, double value) {
    if(!outfile_id) throw std::logic_error("Cannot write attribute " + table + ":" + attrname + " without file");
    herr_t err = H5LTset_attribute_double(outfile_id, table.c_str(), attrname.c_str(), &value, 1);
    if(err < 0) throw std::runtime_error("H5LTset_attribute_double error setting attribute " + table + ":" + attrname);
}

void HDF5_OutputFile::writeAttribute(const string& table, const string& attrname, const string& value) {
    if(!outfile_id) throw std::logic_error("Cannot write attribute " + table + ":" + attrname + " without file");
    herr_t err = H5LTset_attribute_string(outfile_id, table.c_str(), attrname.c_str(), value.c_str());
    if(err < 0) throw std::runtime_error("H5LTset_attribute_string error " + table + ":" + attrname);
}
