#!/bin/bash

# use '**' for directory recursion
shopt -s globstar

for f0 in **/*; do :

if [[ ! -f $f0 ]]; then
continue; fi

p=`dirname "$f0"`
f=`basename "$f0"`
ext="${f##*.}"

if [ "$f" == "Makefile" ] || [ "$f" == "CMakeLists.txt" ] || [ "$f" == "__pycache__" ] || [ "$ext" == "pyc" ] \
|| [ "$f" == "LinkDef.h" ] || [  "$f" == "Doxyfile" ] || [  "$f" == "README.txt" ]; then
continue; fi

showdiff() {
    echo "-----------------------------------------------------------------------------------------------------------------------------------" $f
    if ! diff "$f0" "$1"; then
    echo
    echo "cp \"$f0\" \"$1\""
    echo "--------------------------------------------------------- [ $f ] ------------------------------------------------------------------"
    echo "cp \"$1\" \"$f0\""
    echo
    fi
}

if [ -e "$1/$p/$f" ]; then
    showdiff "$1/$p/$f"
else
    for d1 in $1/* $1/$p/; do :
    if [ -e "$d1/$f" ]; then
        showdiff "$d1/$f"; fi
    done
fi


done

echo "< : here; > : $1"

