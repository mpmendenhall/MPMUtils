#!/bin/bash

for f in */*; do :
if [ -e $MPMUTILS/$f ]; then
    echo "--------------------------------------------------------" $f
    if ! diff $f $MPMUTILS/$f; then
    echo "cp" $f $MPMUTILS/$f; fi
fi; done
echo "< : here; > : MPMUtils"
