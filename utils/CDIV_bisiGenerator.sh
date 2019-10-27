#!/bin/bash

START=$1
END=99

for (( i=$START; i<=$END; i++ ))
do
    echo -e '#!/bin/bash\n\n' > c${i}/s.sh
    ls c${i}/c200_* > c${i}/list
    cat c${i}/list | sed -r 's/c200_Redde.top_//' | sed -r 's/c200_Redde_//'| sed -r 's/c200_GAVG_//' | sed -r 's/c200_CRCSExp_//' | sed -r 's/c200_CRCSLin_//'  | sed -r "s/_c${i}//" > c${i}/list1
    awk '{print "mv -f "$0" "}' c${i}/list > c${i}/temp1
    paste -d " " c${i}/temp1 c${i}/list1 > c${i}/temp2
    awk '{print ""$0" &&"}' c${i}/temp2 >> c${i}/s.sh
    echo -e "\necho 'done c${i}'\n" >> c${i}/s.sh
    rm c${i}/temp1
    rm c${i}/temp2
    rm c${i}/list1
    rm c${i}/list
    chmod +x c${i}/s.sh
    ./c${i}/s.sh
    ls c${i}/q* > c${i}/list
    cat `cat c${i}/list` >> c${i}_qall
    number=`wc -l c${i}/list | cut -f1 -d' '`
    mv c${i}_qall c${i}_qall${number}
done

