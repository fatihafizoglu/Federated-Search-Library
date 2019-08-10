#!/bin/bash

RESULT=$1

START=0
END=99

for (( i=$START; i<=$END; i++ ))
do
    pcregrep -M "\tfs\t${i}$" ${RESULT} > ${RESULT}_c${i}
    pcregrep -M "\tfs\t${i}$" ${RESULT} >> ${RESULT}_call
done
