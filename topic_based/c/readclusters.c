#include <stdio.h>
#include <stdlib.h>

/* prints document ids of a cluster. */
int main (int argc, char *argv[]) {
    FILE *fp;
    int value;
    
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
