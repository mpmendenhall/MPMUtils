/// \file HDF5_StructInfo.hh Struct layout information for HDF5 tables

#ifndef HDF5_STRUCTINFO_HH
#define HDF5_STRUCTINFO_HH

#include "hdf5.h"
#include "hdf5_hl.h"
#include <string>
using std::string;

/// info for setting up HDF5 tables
struct HDF5_Table_Spec {
    hsize_t n_fields;           ///< number of fields
    size_t struct_size;         ///< size of struct
    const size_t* offsets;      ///< field offsets
    const size_t* field_sizes;  ///< field sizes
    const hid_t* field_types;   ///< field data types
    const char** field_names;   ///< field names
    string table_name;          ///< table name
    string table_descrip;       ///< description string for table
};

/// Example for StructInfo setup
struct exampleStruct {
    int foo;
    double bar;
};

/// set up specified table
void makeTable(const HDF5_Table_Spec& T, hid_t outfile_id, int nchunk, int compress);

/// Class/namespace providing struct information for HDF5 table structures
class HDF5_StructInfo {
public:
    // float[2] array type
    static hsize_t const array_dim_2;
    static hid_t const float2_tid;

    // double[3] array type
    static hsize_t const array_dim_3;
    static hid_t const vec3_tid;

    /// exampleStruct setup
    static HDF5_Table_Spec Example_table_setup();
};

/// templatized form for table setup lookup. Non-specialized version should cause compiler barf.
template<typename T>
inline HDF5_Table_Spec HDF5_table_setup() { return T::unimplemented_function(); }
template<>
inline HDF5_Table_Spec HDF5_table_setup<exampleStruct>() { return HDF5_StructInfo::Example_table_setup(); }

#endif
