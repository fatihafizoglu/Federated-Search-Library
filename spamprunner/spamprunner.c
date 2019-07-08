#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <math.h>

void removeSpam(FILE* src, FILE* dest, int threshold, unsigned char* spamScores) {
    int query, docId, rank, prevQuery = 0, prevRank = 0;
    float score;

    while (fscanf(src, "%d\tQ0\t%d\t%d\t%f\tfs\n", &query, &docId, &rank, &score) != EOF) {
        if (query != prevQuery) {
            prevQuery = query;
            prevRank = 0;
        }

        if (spamScores[docId] >= threshold) {
            fprintf(dest, "%d\tQ0\t%d\t%d\t%2.6f\tfs\n", query, docId, rank, score);
        }
    }
}

void usage() {
    printf("USAGE: spampruner sourceRanking destRanking spamFilePath spamScore");
}

int main (int argc, char **argv) {
    unsigned char* scores = (unsigned char*)malloc(50220539);
    FILE *src;
    FILE *dest;
    FILE* spam;

    if(argc != 5) {
      usage();
      return -1;
    }

    src = fopen(argv[1], "r");
    dest = fopen(argv[2], "w");

    spam = fopen(argv[3], "r");

    fread(scores, 1, 50220539, spam);
    fclose(spam);

    removeSpam(src, dest, atoi(argv[4]), scores);

    fclose(src);
    fclose(dest);

    return 0;
}
