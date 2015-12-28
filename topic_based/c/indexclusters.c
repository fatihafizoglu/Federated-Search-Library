#include <stdio.h>
#include <stdlib.h>

/*
 * Prints usage information.
 */
void usage () {
        printf("Usage:\n"
               "\tindexclusters [OPTION]\n"
               "\nDescription:\n"
               "\tCreate indexes of given cluster(s).\n"
               "\nOptions:\n"
               "\t-n [COUNT]\n"
               "\t-c [CLUSTER_ID]\n");
}

int main (int argc, char *argv[]) {
    if (argc < 2) {
        usage();
        exit(EXIT_SUCCESS);
    }




    return 0;
}
