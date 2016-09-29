#!/bin/bash

############################################################################
##
##  Put diversify, ndeval executables or resource_selection.py file on the
##  same folder with this script.
##  
##  Functions:
##      * diversify_CSI             [preresult(200) to result(100)]
##      * diversify_Rs              [preresult(100) to result(20)]
##      * diversify_DivRs           [preresult(100) to result(20)]
##      * resourceSelection_CSI
##      * resourceSelection_Div
##      * evaluate_RsDiv 
##      * evaluate_DivRsDiv
##
##  Note: Build diversify according to reference numbers.
##
#############################################################################

CSI_Top500_NoSpamFixed="c500nsf"
MAIN_Top100k_NoSpamFixed="m100knsf"

Doc2Cluster_Map="doc_sorted"
Ground_Truth="ground_truth-2009-withA.txt"

rsAlgorithms=(
    "CRCSExp"
    "CRCSLin"
    "GAVG"
    "Redde"
    "Redde.top"
)

divAlgorithms=(
    "maxsum0.25"
    "maxsum0.50"
    "maxsum0.75"
    "maxsum1.00"
    "sf0.25"
    "sf0.50"
    "sf0.75"
    "sf1.00"
)

execute () {
    length=$(($#-1))
    command=${@:1:$length}

    if [ -z "$2" ]
    then
        echo "$* >/dev/null"
    else
        echo "$command >$2"
    fi

    if [ $? -ne 0 ]; then
        echo
        echo ">> ERROR: executing $*"
        echo
        exit 1
    fi
}

getMergedFilenames() {
    declare -a filenames=()
    declare -a argArr1=("${!1}")
    declare -a argArr2=("${!2}")
    
    for i in ${argArr1[@]}; do
        for j in ${argArr2[@]}; do
            filenames[${#filenames[@]}]="${i}_${j}"
        done
    done
    
    echo ${filenames[@]}
}

diversifyList() {
    declare -a argArr=("${!1}")
      
    for i in ${argArr[@]}; do
       execute "./diversify ${i}"
    done
}

resourceSelectList() {
    declare -a argArr=("${!1}")

    for i in ${argArr[@]}; do
        for alg in ${rsAlgorithms[@]}; do
            execute "python resource_selection.py ${MAIN_Top100k_NoSpamFixed} ${i} ${Doc2Cluster_Map} -method ${alg}"
        done
    done
}

evaluateList() {
    declare -a argArr=("${!1}")

    for i in ${argArr[@]}; do
       execute "./ndeval ${Ground_Truth} ${i}" "${i}_eval"
    done
}

diversify_CSI () {
    diversifyList CSI_Top500_NoSpamFixed       
}

diversify_Rs () {
    declare -a rsFiles=()
    rsFiles=$(getMergedFilenames CSI_Top500_NoSpamFixed rsAlgorithms[@])   
    diversifyList rsFiles[@]
}

diversify_DivRs () {
    declare -a divFiles=()
    divFiles=$(getMergedFilenames CSI_Top500_NoSpamFixed divAlgorithms[@])
    declare -a divRsFiles=()
    divRsFiles=$(getMergedFilenames divFiles rsAlgorithms[@])
    diversifyList divRsFiles[@]
}

resourceSelection_CSI () {
    resourceSelectList CSI_Top500_NoSpamFixed
}

resourceSelection_Div () {
    declare -a divFiles=()
    divFiles=$(getMergedFilenames CSI_Top500_NoSpamFixed divAlgorithms[@])
    resourceSelectList divFiles[@]
}

evaluate_RsDiv () {
    declare -a rsFiles=()
    divFiles=$(getMergedFilenames CSI_Top500_NoSpamFixed rsAlgorithms[@])
    declare -a rsDivFiles=()
    rsDivFiles=$(getMergedFilenames divFiles divAlgorithms[@])
    evaluateList rsDivFiles[@]
}

evaluate_DivRsDiv () {
    declare -a divFiles=()
    divFiles=$(getMergedFilenames CSI_Top500_NoSpamFixed divAlgorithms[@])
    declare -a divRsFiles=()
    divRsFiles=$(getMergedFilenames divFiles rsAlgorithms[@])
    declare -a divRsDivFiles=()
    divRsDivFiles=$(getMergedFilenames divRsFiles divAlgorithms[@])
    evaluateList divRsDivFiles[@]
}

#diversify_CSI
#diversify_Rs
#diversify_DivRs
#resourceSelection_CSI
#resourceSelection_Div
#evaluate_RsDiv
#evaluate_DivRsDiv
