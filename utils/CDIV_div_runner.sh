#!/bin/bash

# ./diversify c200_Redde.top_q21_c34 /home1/grupef/ecank/data/CLUSTERS/wordlist_34

preresults_prefix=$1
#preresults_prefix="c200_"
#preresults_prefix="c200_CRCSExp_q"
#preresults_prefix="c200_CRCSLin_q"
#preresults_prefix="c200_GAVG_q"
#preresults_prefix="c200_Redde_q"
#preresults_prefix="c200_Redde.top_q"
wordlist_prefix="/home1/grupef/ecank/data/CLUSTERS/wordlist_"

for entry in "${preresults_prefix}"*
do
    cluster_id=`echo $entry | grep -Eo '[0-9]+$'`
    #echo "./diversify ${entry} ${wordlist_prefix}${cluster_id}"
    ./diversify ${entry} ${wordlist_prefix}${cluster_id}
done
