#!/bin/bash

# use '**' for directory recursion
shopt -s globstar

for f0 in **/*; do :

if [[ ! -f $f0 ]]; then
continue; fi

f=`basename "$f0"`
ext="${f##*.}"

if [ "$f" == "Makefile" ] || [ "$f" == "CMakeLists.txt" ] || [ "$f" == "__pycache__" ] || [ "$ext" == "pyc" ] \
|| [ "$f" == "HDF5_StructInfo.hh" ] || [ "$f" == "HDF5_StructInfo.cc" ] || [ "$f" == "LinkDef.h" ]; then
continue; fi
for d1 in $1/*; do :
if [ -e "$d1/$f" ]; then
    echo "--------------------------------------------------------" $f
    if ! diff $f0 $d1/$f; then
    echo "cp" $f0 $d1/$f
    echo "cp" $d1/$f $f0
    fi
fi; done; done

echo "< : here; > : $1"

