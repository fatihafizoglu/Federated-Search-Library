#include <stdio.h>

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
    
    return 0;
}
