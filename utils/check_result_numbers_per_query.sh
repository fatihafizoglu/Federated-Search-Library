#!/bin/bash

results_file=$1
number_of_query=$2
expected_number_of_results=$3

echo "$0 results_file #query #expected_results_per_query"
echo "e.g. $0 c200 198 200"
echo "You executed: $0 $results_file $number_of_query $expected_number_of_results"

i="1"
while [ $i -le $number_of_query ]
do
    number_of_results=`grep -zoP "\n$i\tQ0" $results_file | wc -l`
    if [ $i -eq 1 ]
    then
        number_of_results=$((number_of_results+1))
    fi

    if [ $number_of_results -ne $expected_number_of_results ]
    then
        echo "$i:$number_of_results"
    fi

    i=$[$i+1]
done
