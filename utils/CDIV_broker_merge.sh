#!/bin/bash

Q_C_MAP=$1

START=1 #1
END=198 #198

div_methods=(maxsum0.25
            maxsum0.50
            maxsum0.75
            sf0.25
            sf0.50
            sf0.75)

for (( i=$START; i<=$END; i++ ))
do
    mkdir c200_rs_cdiv_qs/q${i}
    echo "Merging query#${i} results...\n"
    cs=`pcregrep -M "^${i} " $Q_C_MAP | cut -f2 -d" "`
    clusters=($cs)
    echo "\tCollecting "
    for cluster in ${clusters[@]}; do
        internal_query_number=`awk "/q$i/{ print NR; exit }" cs/c${cluster}/list`
        echo " C#$cluster(q#$internal_query_number)."
        for dm in ${div_methods[@]}; do
            query_count=`ls cs/c${cluster}/q* | wc -l`
            result_file="cqs/c${cluster}_qall${query_count}_${dm}"
            echo "Looking: " $result_file
            pcregrep -M "^${internal_query_number}\tQ0\t" $result_file | while read -r line ; do
                echo $line >> c200_rs_cdiv_qs/q${i}/${Q_C_MAP}_${dm}
            done
        done
    done

done
