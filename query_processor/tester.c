#include "qp_bm25.h"

int main(int argc,char *argv[]) {

    if (load_subqueries("/home/ecank/Desktop/SUBTOPICS-2009-50-reprocessed.txt") != 0) {
        printf("load_subqueries failed.\n");
        exit(1);
    }



    return 0;
}
