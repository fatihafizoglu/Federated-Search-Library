#!/bin/bash

START=2
END=100

for (( i=$START; i<=$END; i++ ))
do
    mkdir c${i} && mv c200_*c${i} c${i}/.
done
