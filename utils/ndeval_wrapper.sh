#!/bin/bash

## e.g. ./ndeval_wrapper.sh results/pool/*

Ground_Truth="/home1/grupef/ecank/data/GT_TREC09101112.txt"

for result in "$@"
do
    ./ndeval ${Ground_Truth} ${result} > ${result}_eval
done

source ./run.sh
generateEvalTables_BASELINE
generateEvalTables_BDIV
generateEvalTables_DDIV
generateEvalTables_DDIVPP
