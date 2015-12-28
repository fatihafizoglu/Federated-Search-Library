#include <stdio.h>
#include <stdlib.h>

/*
 * Prints usage information.
 */
void usage () {
        printf("Usage:\n"
               "\treadclusters [FILE]\n"
               "\nDescription:\n"
               "\tPrints document ids of a given cluster.\n");
}

/* prints document ids of a cluster. */
int main (int argc, char *argv[]) {
    FILE *fp;
    int value;

    if (argc < 2) {
        usage();
        exit(EXIT_SUCCESS);
    }

    if (!(fp = fopen(argv[1], "r"))) {
        printf("fopen problem\n");
        return -1;
    }

    while (fread(&value, 1, 4, fp)) {
        printf("%d\n", value);
    }

    fclose(fp);

    return 0;
}
