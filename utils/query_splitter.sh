#!/bin/bash

QUERIES="/home1/grupef/ecank/data/querylist_TREC09101112.txt"
OUTPUT="/home1/grupef/ecank/data/TREC/i/query"

i="1"
while read p; do
    echo "$p" > ${OUTPUT}_${i}
    i=$[$i+1]
done <$QUERIES
