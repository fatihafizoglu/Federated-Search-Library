#!/bin/bash

Q_C_MAP=$1

START=1
END=1 #198

div_methods=(maxsum0.25 
            maxsum0.50 
            maxsum0.75 
            sf0.25 
            sf0.50 
            sf0.75)

for (( i=$START; i<=$END; i++ ))
do
    printf "Merging query#${i} results...\n"
    cs=`pcregrep -M "\n${i} " $Q_C_MAP | cut -f2 -d" "`
    clusters=($cs)
    printf "\tCollecting "
    for cluster in ${clusters[@]}; do 
        internal_query_number=`awk "/q$i/{ print NR; exit }" cs/c${cluster}/list`
        printf " C#$cluster(q#$internal_query_number)."
        for dm in ${div_methods[@]}; do
            query_count=`ls cs/c${cluster}/q* | wc -l`
            result_file="cqs/c${cluster}_qall${query_count}_${dm}"
            results=`pcregrep -M "\n${internal_query_number}\tQ0\t" $result_file`
            echo $results >> ${Q_C_MAP}_${dm}
        done
        
        
        
    done
    printf "\tDone.\n"
    
done


