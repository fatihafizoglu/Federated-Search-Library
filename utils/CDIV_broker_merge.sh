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

rs_methods=(CRCSExp
            CRCSLin
            GAVG
            Redde
            Redde.top)

for (( i=$START; i<=$END; i++ ))
do
    mkdir c200_rs_cdiv_qs/q${i}
done

echo $Q_C_MAP
for (( i=$START; i<=$END; i++ ))
do
    echo "Merging query#${i}"
    cs=`pcregrep -M "^${i} " $Q_C_MAP | cut -f2 -d" "`
    clusters=($cs)
    for cluster in ${clusters[@]}; do
        internal_query_number=`awk "/q$i/{ print NR; exit }" cs/c${cluster}/list`
        printf "C$cluster(q#$internal_query_number) "
        for dm in ${div_methods[@]}; do
            query_count=`ls cs/c${cluster}/q* | wc -l`
            result_file="cqs/c${cluster}_qall${query_count}_${dm}"
            #echo "Looking: " $result_file
            pcregrep -M "^${internal_query_number}\tQ0\t" $result_file >> c200_rs_cdiv_qs/q${i}/${Q_C_MAP}_${dm}
        done
    done
    echo "Done!"
done

# Fix query numbers
for (( i=$START; i<=$END; i++ ))
do
    sed -i "s/[[:digit:]]\+\tQ0/${i}\tQ0/g" c200_rs_cdiv_qs/q${i}/*
done

# TODO: Retrieve scores from main index


# Sort results wrt their scores
for entry in c200_rs_cdiv_qs/q*/*
do
    sort -rg -k5,5 ${entry} -o ${entry}_sorted
    sed -i '21,$ d' ${entry}_sorted
done

# Merge queries
for (( i=$START; i<=$END; i++ ))
do
    for rs in ${rs_methods[@]}; do
        for div in ${div_methods[@]}; do
            cat c200_rs_cdiv_qs/q${i}/c200_${rs}_${div}_sorted >> c200_rs_cdiv_results/${rs}_${div}
        done
    done
done

# Fix result ranks
for rs in ${rs_methods[@]}; do
    for div in ${div_methods[@]}; do
        ./result_rank_fixer c200_rs_cdiv_results/${rs}_${div}
    done
done

# Evaluate results
for rs in ${rs_methods[@]}; do
    for div in ${div_methods[@]}; do
        ./ndeval GT_TREC09101112.txt c200_rs_cdiv_results/${rs}_${div}_fixed > c200_rs_cdiv_results/${rs}_${div}_eval
    done
done
