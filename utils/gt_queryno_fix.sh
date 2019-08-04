#!/bin/bash

# I assume query numbers start from 1. And the last one is:
LAST=48
DIFF=50 # So, e.g. query no:1 now will be 1+DIFF
IN="TREC/2010.txt"
OUT="${IN}_fixed"

cp $IN $OUT
# When DIFF < LAST, unwanted changes might happen.
# That's why start from last.
for (( c=$LAST; c>0; c-- ))
do
    new_queryno=$((c + DIFF))
    find="\n$c "
    replace="\n$new_queryno "

    cmd="sed -i ':a;N;\$!ba;s/$find/$replace/g' $OUT"
    echo $cmd
    eval $cmd
done
