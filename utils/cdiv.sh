#!/bin/bash

RESULT=$1
Ground_Truth="/home/eckucukoglu/Desktop/allcdivs/GT_TREC09101112.txt"


START=0
END=99

for RESULT in "$@"
do
    for (( i=$START; i<=$END; i++ ))
    do
        pcregrep -M "\tfs\t${i}$" ${RESULT} >> ${RESULT}_call
    done
done

## ABOVE PART BEFORE DIV
## BELOW PART AFTER DIV

## e.g. ./cdiv cdivrun/*
#
# START=1
# END=198
# CDIV_EVAL="CDIV_eval"
# echo "method,file,runid,topic,ERR-IA@5,ERR-IA@10,ERR-IA@20,nERR-IA@5,nERR-IA@10,nERR-IA@20,alpha-DCG@5,alpha-DCG@10,alpha-DCG@20,alpha-nDCG@5,alpha-nDCG@10,alpha-nDCG@20,NRBP,nNRBP,MAP-IA,P-IA@5,P-IA@10,P-IA@20,strec@5,strec@10,strec@20" > ${CDIV_EVAL}
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
#     ./ndeval ${Ground_Truth} ${RESULT}f > ${RESULT}f_eval
#     line=$(tail -1 ${RESULT}f_eval)
#     echo "CDIV",${RESULT},$line >> ${CDIV_EVAL}
# done
