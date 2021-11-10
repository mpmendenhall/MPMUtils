/// \file HDF5_StructInfo.cc
// -- Michael P. Mendenhall, LLNL 2019

#include "HDF5_StructInfo.hh"
#include <stdexcept>

hsize_t const array_dim_2 = 2;
hsize_t const array_dim_3 = 3;
hid_t const float2_tid = H5Tarray_create(H5T_NATIVE_FLOAT, 1, &array_dim_2);
hid_t const float3_tid = H5Tarray_create(H5T_NATIVE_FLOAT, 1, &array_dim_3);
hid_t const double3_tid = H5Tarray_create(H5T_NATIVE_DOUBLE, 1, &array_dim_3);

void makeTable(const HDF5_Table_Spec& T, hid_t outfile_id, int nchunk, int compress) {
    if(!outfile_id) throw std::runtime_error("No HDF5 output file specified");
    printf("Setting up '%s' table...\n", T.table_name.c_str());
    herr_t err = H5TBmake_table(T.table_descrip.c_str(), outfile_id, T.table_name.c_str(),
                                T.n_fields, 0, T.struct_size,
                                T.field_names, T.offsets,T.field_types,
                                nchunk, nullptr, compress, nullptr);
    if(err<0) throw std::runtime_error("Error instantiating HDF5 table");
}
