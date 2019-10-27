#!/bin/bash

DOC_TO_CLUSTER_MAP="/home1/grupef/ecank/data/doc_to_cluster_map"
DOC_ID=$1

line=`sed "${DOC_ID}q;d" $DOC_TO_CLUSTER_MAP`
linearray=($line)
if [ ${linearray[0]} -ne ${DOC_ID} ]; then
    echo -e "Query:\tD# ${DOC_ID}"
    echo -e "Result:\tD# ${linearray[0]} C# ${linearray[1]}"
    exit 1
else
    echo ${linearray[1]}
fi
