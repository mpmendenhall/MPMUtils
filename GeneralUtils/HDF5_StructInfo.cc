/// \file HDF5_StructInfo.cc

#include "HDF5_StructInfo.hh"
#include "HDF5_Table_Cache.hh"
#include <exception>

template<>
int64_t HDF5_Table_Cache<exampleStruct>::getIdentifier(const exampleStruct& val) { return val.foo; }
template<>
void  HDF5_Table_Cache<exampleStruct>::setIdentifier(exampleStruct& val, int64_t newID) { val.foo = newID; }

/////////////////////////////////
/////////////////////////////////
/////////////////////////////////

hsize_t const HDF5_StructInfo::array_dim_3 = 3;
hid_t const HDF5_StructInfo::vec3_tid = H5Tarray_create(H5T_NATIVE_DOUBLE, 1, &HDF5_StructInfo::array_dim_3);

hsize_t const HDF5_StructInfo::array_dim_2 = 2;
hid_t const HDF5_StructInfo::float2_tid = H5Tarray_create(H5T_NATIVE_FLOAT, 1, &HDF5_StructInfo::array_dim_2);

/////////////////////////////////
/////////////////////////////////
/////////////////////////////////

HDF5_Table_Spec HDF5_StructInfo::Example_table_setup() {

    const hsize_t n_fields = 2;

    static const size_t offsets[n_fields] = {
        HOFFSET(exampleStruct, foo),
        HOFFSET(exampleStruct, bar)
    };

    static size_t const field_sizes[n_fields] = {
        sizeof(exampleStruct::foo),
        sizeof(exampleStruct::bar)
    };

    static const char* field_names[n_fields] = { "foo", "bar" };

    static hid_t const field_types[n_fields] = { H5T_NATIVE_INT, H5T_NATIVE_DOUBLE };

    HDF5_Table_Spec T;
    T.n_fields = n_fields;
    T.struct_size = sizeof(exampleStruct);
    T.offsets = offsets;
    T.field_sizes = field_sizes;
    T.field_names = field_names;
    T.field_types = field_types;
    T.table_name = "exampleStruct";
    T.table_descrip = "Example table setup";
    return T;
}

/////////////////////////////////
/////////////////////////////////
/////////////////////////////////

void makeTable(const HDF5_Table_Spec& T, hid_t outfile_id, int nchunk, int compress) {
    assert(outfile_id);
    printf("Setting up '%s' table...\n", T.table_name.c_str());
    herr_t err = H5TBmake_table(T.table_descrip.c_str(), outfile_id, T.table_name.c_str(),
                                T.n_fields, 0, T.struct_size,
                                T.field_names, T.offsets,T.field_types,
                                nchunk, nullptr, compress, nullptr);
    if(err<0) throw std::exception();
}
