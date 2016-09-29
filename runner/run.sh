#!/bin/bash

#############################################################################
## README ##
#############################################################################
##
##  What it is?
##      - This script has some functionalities to diversify, make resouce
##        selection and evaluation from query result files. Moreover, 
##        evaluation tables can be generated easily.
##
##  How to run?
##      - Get diversify(c), resource selection(python) and Trec Web 
##        Evaluation(c) codes.
##      - Build diversify according to reference result numbers below.
##      - Build Trec Web Evaluation (ndeval).
##      - Put diversify, ndeval executables or resource_selection.py file 
##        on the same folder with this script. 
##      - Change the 'paths' -they are not filenames- from below.
##      - Comment-out the function from end of code that you want to use.
##  
##  Functions:
##      * diversify_CSI             [preresult(200) to result(100)]
##      * diversify_Rs              [preresult(100) to result(20)]
##      * diversify_DivRs           [preresult(100) to result(20)]
##      * resourceSelection_CSI
##      * resourceSelection_Div
##      * evaluate_RsDiv 
##      * evaluate_DivRsDiv
##      * generateEvalTables_RsDiv
##      * generateEvalTables_DivRsDiv
##
##  Note: 
##        
##
#############################################################################
## PATHS ##
#############################################################################
CSI_Top500_NoSpamFixed="c500nsf"
MAIN_Top100k_NoSpamFixed="m100knsf"
Doc2Cluster_Map="doc_sorted"
Ground_Truth="ground_truth-2009-withA.txt"
#############################################################################
## Possible options ##
#############################################################################
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
#############################################################################
## Functions ## Do not change below functions.
#############################################################################
execute () {
    length=$(($#-1))
    command=${@:1:$length}

    if [ -z "$2" ]
    then
        $* >/dev/null
    else
        $command >$2
    fi

    if [ $? -ne 0 ]; then
        echo
        echo ">> ERROR: executing $*"
        echo
        exit 1
    fi
}

getMergedFilenames() {
    local -a filenames=()
    local -a argArr1=("${!1}")
    local -a argArr2=("${!2}")
    
    for i in ${argArr1[@]}; do
        for j in ${argArr2[@]}; do
            filenames[${#filenames[@]}]="${i}_${j}"
        done
    done
    
    echo ${filenames[@]}
}

diversifyList() {
    local -a argArr=("${!1}")
      
    for i in ${argArr[@]}; do
       execute "./diversify ${i}"
    done
}

resourceSelectList() {
    local -a argArr=("${!1}")

    for i in ${argArr[@]}; do
        for alg in ${rsAlgorithms[@]}; do
            execute "python resource_selection.py ${MAIN_Top100k_NoSpamFixed} ${i} ${Doc2Cluster_Map} -method ${alg}"
        done
    done
}

evaluateList() {
    local -a argArr=("${!1}")

    for i in ${argArr[@]}; do
       execute "./ndeval ${Ground_Truth} ${i}" "${i}_eval"
    done
}

diversify_CSI () {
    diversifyList CSI_Top500_NoSpamFixed       
}

diversify_Rs () {
    local -a rsFiles=()
    rsFiles=$(getMergedFilenames CSI_Top500_NoSpamFixed rsAlgorithms[@])   
    diversifyList rsFiles[@]
}

diversify_DivRs () {
    local -a divFiles=()
    divFiles=$(getMergedFilenames CSI_Top500_NoSpamFixed divAlgorithms[@])
    local -a divRsFiles=()
    divRsFiles=$(getMergedFilenames divFiles rsAlgorithms[@])
    diversifyList divRsFiles[@]
}

resourceSelection_CSI () {
    resourceSelectList CSI_Top500_NoSpamFixed
}

resourceSelection_Div () {
    local -a divFiles=()
    divFiles=$(getMergedFilenames CSI_Top500_NoSpamFixed divAlgorithms[@])
    resourceSelectList divFiles[@]
}

evaluate_RsDiv () {
    local -a rsFiles=()
    divFiles=$(getMergedFilenames CSI_Top500_NoSpamFixed rsAlgorithms[@])
    local -a rsDivFiles=()
    rsDivFiles=$(getMergedFilenames divFiles divAlgorithms[@])
    evaluateList rsDivFiles[@]
}

evaluate_DivRsDiv () {
    local -a divFiles=()
    divFiles=$(getMergedFilenames CSI_Top500_NoSpamFixed divAlgorithms[@])
    local -a divRsFiles=()
    divRsFiles=$(getMergedFilenames divFiles rsAlgorithms[@])
    local -a divRsDivFiles=()
    divRsDivFiles=$(getMergedFilenames divRsFiles divAlgorithms[@])
    evaluateList divRsDivFiles[@]
}

generateEvalTables_RsDiv () {
    local -a rsFiles=()
    divFiles=$(getMergedFilenames CSI_Top500_NoSpamFixed rsAlgorithms[@])
    local -a rsDivFiles=()
    rsDivFiles=$(getMergedFilenames divFiles divAlgorithms[@])
    local -a rsDivEvalFiles=()
    local -a eval_suffix=("eval")
    rsDivEvalFiles=$(getMergedFilenames rsDivFiles eval_suffix)
    
    rsDiv_eval_file="rsDiv_eval"
    echo "file,runid,topic,ERR-IA@5,ERR-IA@10,ERR-IA@20,nERR-IA@5,nERR-IA@10,nERR-IA@20,alpha-DCG@5,alpha-DCG@10,alpha-DCG@20,alpha-nDCG@5,alpha-nDCG@10,alpha-nDCG@20,NRBP,nNRBP,MAP-IA,P-IA@5,P-IA@10,P-IA@20,strec@5,strec@10,strec@20" > ${rsDiv_eval_file}
    
    for i in ${rsDivEvalFiles[@]}; do
       line=$(tail -1 $i)
       echo $i,$line >> ${rsDiv_eval_file}
    done
}

generateEvalTables_DivRsDiv () {
    local -a divFiles=()
    divFiles=$(getMergedFilenames CSI_Top500_NoSpamFixed divAlgorithms[@])
    local -a divRsFiles=()
    divRsFiles=$(getMergedFilenames divFiles rsAlgorithms[@])
    local -a divRsDivFiles=()
    divRsDivFiles=$(getMergedFilenames divRsFiles divAlgorithms[@])
    local -a divRsDivEvalFiles=()
    local -a eval_suffix=("eval")
    divRsDivEvalFiles=$(getMergedFilenames divRsDivFiles eval_suffix)
    
    divRsDiv_eval_file="divRsDiv_eval"
    echo "file,runid,topic,ERR-IA@5,ERR-IA@10,ERR-IA@20,nERR-IA@5,nERR-IA@10,nERR-IA@20,alpha-DCG@5,alpha-DCG@10,alpha-DCG@20,alpha-nDCG@5,alpha-nDCG@10,alpha-nDCG@20,NRBP,nNRBP,MAP-IA,P-IA@5,P-IA@10,P-IA@20,strec@5,strec@10,strec@20" > ${divRsDiv_eval_file}
    
    for i in ${divRsDivEvalFiles[@]}; do
       line=$(tail -1 $i)
       echo $i,$line >> ${divRsDiv_eval_file}
    done
}

#############################################################################
## Main ## Comment-out the function you want to use
#############################################################################
#diversify_CSI
#diversify_Rs
#diversify_DivRs
#resourceSelection_CSI
#resourceSelection_Div
#evaluate_RsDiv
#evaluate_DivRsDiv
#generateEvalTables_RsDiv
#generateEvalTables_DivRsDiv
