#!/bin/bash

START=$1
END=$2
LOG="log"
# index_generate: msworks/index_splitter_for_sampled_documents.c
for (( i=$START; i<=$END; i++ ))
do
    ./index_generate data/inverted_index data/wordlist data/CLUSTERS/docs/cluster$i data/CLUSTERS/inverted_index_$i data/CLUSTERS/wordlist_$i >> ${LOG}_${START}_${END}_${i}
    sleep 2
done
