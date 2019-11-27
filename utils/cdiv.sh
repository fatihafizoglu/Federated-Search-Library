#!/bin/bash

# RESULT=$1
#
# START=0
# END=99
#
# # for RESULT in "$@"
# # do
# for (( i=$START; i<=$END; i++ ))
# do
#     pcregrep -M "\tfs\t${i}$" ${RESULT} >> ${RESULT}_call
# done
# # done
#
# ## ABOVE PART BEFORE CDIV
# ## BELOW PART AFTER CDIV
#
# ## e.g. ./cdiv cdivrun/*
# START=1
# END=198
#
# for RESULT in "$@"
# do
#     for (( i=$START; i<=$END; i++ ))
#     do
#         pcregrep -M "^${i}\tQ0\t" ${RESULT} > ${RESULT}_${i}
#         python sort.py ${RESULT}_${i} ${RESULT}_sorted
#         rm ${RESULT}_${i}
#     done
#
#     ./result_rank_fixer ${RESULT}_sorted
#     rm ${RESULT}_sorted
#     mv ${RESULT}_sorted_fixed ${RESULT}f
# done
