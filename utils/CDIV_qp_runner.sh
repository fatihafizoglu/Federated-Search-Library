#!/bin/bash

INPUT="$1"
#INPUT="/home1/grupef/ecank/results/CDIV/c200_CRCSExp"
#INPUT="/home1/grupef/ecank/results/CDIV/c200_Redde"
#INPUT="/home1/grupef/ecank/results/CDIV/c200_CRCSLin"
#INPUT="/home1/grupef/ecank/results/CDIV/c200_Redde.top"
#INPUT="/home1/grupef/ecank/results/CDIV/c200_GAVG"

WORDLIST="/home1/grupef/ecank/data/CLUSTERS/wordlist"
INVERTED_INDEX="/home1/grupef/ecank/data/CLUSTERS/inverted_index"
DOC_LENGTHS="/home1/grupef/ecank/data/doc_lengths"
QUERY_FILE="/home1/grupef/ecank/data/TREC/i/query"
OUTPUT_RESULTS_FILE=$INPUT
NOW=$(date +"%Y%m%d%H%M%S")

while read p; do
    IFS=', ' read -r -a array <<< "$p"
    echo "./query ${WORDLIST}_${array[1]}_IDF ${INVERTED_INDEX}_${array[1]} ${DOC_LENGTHS} ${QUERY_FILE}_${array[0]} ${OUTPUT_RESULTS_FILE}_q${array[0]}_c${array[1]}" >> ${0}_${NOW}.log
    ./query ${WORDLIST}_${array[1]} ${INVERTED_INDEX}_${array[1]} ${DOC_LENGTHS} ${QUERY_FILE}_${array[0]} ${OUTPUT_RESULTS_FILE}_q${array[0]}_c${array[1]} >> ${0}_${NOW}.log
done <$INPUT

######################
### 5 ayri dosya icin calisacak bu script.
### Her calistiginda okuyacagi input dosyasinda; o rs metodu icin
### her query icin (198) secilen clusterlarin (10) listesi kadar
### query process edecek. Her seferinde 1 query isleyecek ve top 200
### sonucu donecek. Query processor her calistiginda ~2min is yapiyor olsa
### Bombaya hazir misin? => 1980 * 2 min ~= 33h calisacak demektir.
### ~14gb ram kullaniyor. 9 paralel run mumkun. 5 yeterli.
### BOL SANS!
######################
