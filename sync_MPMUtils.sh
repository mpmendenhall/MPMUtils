#!/bin/bash

# use '**' for directory recursion
shopt -s globstar

for f0 in **/*; do :
f=`basename $f0`
ext="${f##*.}"

if [ "$f" == "Makefile" ] || [ "$f" == "__pycache__" ] || [ "$ext" == "pyc" ] || [ "$f" == "HDF5_StructInfo.hh" ] || [ "$f" == "HDF5_StructInfo.cc" ]; then
continue; fi
for d1 in $MPMUTILS/*; do :
if [ -e $d1/$f ]; then
    echo "--------------------------------------------------------" $f
    if ! diff $f0 $d1/$f; then
    echo "cp" $f0 $d1/$f
    echo "cp" $d1/$f $f0
    fi
fi; done; done

echo "< : here; > : MPMUtils"

