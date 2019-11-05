#include "qp_bm25.h"

int load_subqueries(char *sq_filename) {
    printf("%s\n", sq_filename);
    FILE *sq_file;
    unsigned int q_no, q_no_prev = -1;
    unsigned int sq_no = 0;
    char temp[MAX_SQ_LENGTH] = "";

    if (!(sq_file = fopen(sq_filename, "r"))) {
        printf("Subqueries file not found.\n");
        return -1;
    }

    memset(subqueries, 0, NOF_Q*MAX_SQ_PER_Q*MAX_SQ_LENGTH);
    // for (int i = 0; i < NOF_Q; i++) {
    //     for (int j = 0; j < MAX_SQ_PER_Q; j++) {
    //         subqueries[i][j][0] = '\0';
    //     }
    // }

    while (!feof(sq_file)) {
        // printf("BOK\n");
        fscanf (sq_file, "%u\t%s\n", &(q_no), temp);

        if (q_no != q_no_prev) {
            sq_no = 0;
        }

//         strcpy(subqueries[q_no-1][sq_no], temp);
//
        printf("subqueries[%u][%u]: %s\n", q_no-1, sq_no, subqueries[q_no-1][sq_no]);
        fflush(stdout);
//
//         q_no_prev = q_no;
//         sq_no++;
    }

    fclose(sq_file);
    return 0;
}

int main(int argc,char *argv[]) {

    if (load_subqueries("/home/ecank/Desktop/SUBTOPICS-2009-50-reprocessed.txt") != 0) {
        printf("load_subqueries failed.\n");
        exit(1);
    }
}
