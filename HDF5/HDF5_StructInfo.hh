/// \file HDF5_StructInfo.hh Struct layout information for HDF5 tables
// -- Michael P. Mendenhall, LLNL 2019

#ifndef HDF5_STRUCTINFO_HH
#define HDF5_STRUCTINFO_HH

#include "hdf5.h"
#include "hdf5_hl.h"
#include <string>
using std::string;

/// info for setting up HDF5 tables
struct HDF5_Table_Spec {
    int version = 0;                        ///< version number
    hsize_t n_fields = 0;                   ///< number of fields
    size_t struct_size = 0;                 ///< size of struct
    const size_t* offsets = nullptr;        ///< field offsets
    const size_t* field_sizes = nullptr;    ///< field sizes
    const hid_t* field_types = nullptr;     ///< field data types
    const char** field_names = nullptr;     ///< field names
    string table_name;                      ///< table name
    string table_descrip;                   ///< description string for table
};

/// set up specified table
void makeTable(const HDF5_Table_Spec& T, hid_t outfile_id, int nchunk, int compress);

/// templatized form for table setup lookup. Non-specialized version should cause compiler barf.
template<typename T>
inline HDF5_Table_Spec HDF5_table_setup(const string& tname = "", int version = 0) { return T::HDF5_table_setup(tname, version); }

/// float[2] array type
extern hid_t const float2_tid;
/// float[3] array type
extern hid_t const float3_tid;
/// double[3] array type
extern hid_t const double3_tid;

#endif
