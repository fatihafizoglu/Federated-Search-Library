#!/bin/bash

## e.g. ./eval_wrapper.sh pool/*

Ground_Truth_SIM="GT_SIM_TREC09101112.txt"
Ground_Truth_DIV="GT_DIV_TREC09101112.txt"

for result in "$@"
do
    ./trec_eval -q ${Ground_Truth_SIM} ${result} > ${result}_trec
    ./ndeval ${Ground_Truth_DIV} ${result} > ${result}_ndeval
done

# source ./run.sh
# generateEvalTables_BASELINE
# generateEvalTables_BDIV
# generateEvalTables_DDIV
# generateEvalTables_DDIVPP
