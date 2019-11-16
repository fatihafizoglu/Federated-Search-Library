#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <math.h>

/*******************/
/***** CONFIGS *****/
/*******************/
#define ACCESS_INFO "/home1/grupef/ecank/data/CSI/desc_access_counts"
#define PR_INFO "/home1/grupef/ecank/data/CSI/doc-PR-map2019.txt"
#define SPAMSMAP_FILE_FOLDER "/home1/grupef/ecank/data/spamsMap.bin"
#define RANDOM_OUT "/home1/grupef/ecank/data/CSI/nospam_random_csi_docids"
#define ACCESS_OUT "/home1/grupef/ecank/data/CSI/nospam_access_topk_csi_docids"
#define PR_OUT "/home1/grupef/ecank/data/CSI/nospam_pr_topk_csi_docids"

/*********************/
/***** CONSTANTS *****/
/*********************/
#define SPAM_THRESHOLD 60
#define NOF_DOCS 50220539 // actually +1 to be able to use doc_id as an index
#define NOF_SAMPLED_DOCS 502205

/********************/
/***** GLOBALS ******/
/********************/
unsigned char spam_scores[NOF_DOCS];
unsigned char random_docs[NOF_SAMPLED_DOCS];
unsigned char access_docs[NOF_SAMPLED_DOCS];
unsigned char rp_docs[NOF_SAMPLED_DOCS];

/*********************/
/***** FUNCTIONS *****/
/*********************/
int write(char *output_path, unsigned char *docs) {
    int i = 0;
    FILE *fp = fopen(output_path, "w");
    if (fp == NULL) {
        printf("%s fopen failed\n", output_path);
        return -1;
    }

    for (i = 0; i < NOF_SAMPLED_DOCS; i++) {
        if (docs[i] != 0) {
            fprintf(fp, "%d\n", docs[i]);
        }
    }

    fclose(fp);
    return 0;
}

int write_all () {

    if (write(RANDOM_OUT, random_docs) != 0) {
        printf("%s write failed\n", RANDOM_OUT);
        return -1;
    }

    if (write(ACCESS_OUT, access_docs) != 0) {
        printf("%s write failed\n", ACCESS_OUT);
        return -1;
    }

    if (write(PR_OUT, rp_docs) != 0) {
        printf("%s write failed\n", PR_OUT);
        return -1;
    }

    return 0;
}

int cmpfunc (const void *a, const void *b) {
    return ( *(unsigned int*)a - *(unsigned int*)b );
}

void sort_all () {
    qsort(random_docs, NOF_SAMPLED_DOCS, sizeof(unsigned int), cmpfunc);
    qsort(random_docs, NOF_SAMPLED_DOCS, sizeof(unsigned int), cmpfunc);
    qsort(random_docs, NOF_SAMPLED_DOCS, sizeof(unsigned int), cmpfunc);
}

/* Input file format:
 * <doc_id> <pr>
 * 1 0.15
 */
int load_pr () {
    FILE *fp = fopen(PR_INFO, "r");
    if (fp == NULL) {
        printf("%s fopen failed\n", PR_INFO);
        return -1;
    }



    /*
    if (spamScores[docId] >= SPAM_THRESHOLD) {
        // GOOD!
    }
    */

    fclose(fp);
    return 0;
}

/* Input file format:
 * <count> <doc_id> <doc_name>
 * 57207 42710475 clueweb09-en0011-58-19250
 */
int load_access () {
    FILE *fp = fopen(ACCESS_INFO, "r");
    if (fp == NULL) {
        printf("%s fopen failed\n", ACCESS_INFO);
        return -1;
    }

    /*
    if (spamScores[docId] >= SPAM_THRESHOLD) {
        // GOOD!
    }
    */

    fclose(fp);
    return 0;
}

int load_random () {

    /*
    if (spamScores[docId] >= SPAM_THRESHOLD) {
        // GOOD!
    }
    */

    return 0;
}

int load_spam () {
    FILE* fp;

    fp = fopen(SPAMSMAP_FILE_FOLDER, "r");
    if (fp == NULL) {
        printf("%s fopen failed\n", SPAMSMAP_FILE_FOLDER);
        return -1;
    }

    if (fread(spam_scores, 1, NOF_DOCS, fp) != NOF_DOCS) {
        printf("%s fread failed\n", SPAMSMAP_FILE_FOLDER);
        return -1;
    }

    fclose(fp);
    return 0;
}

int load_all () {
    if (load_spam() != 0) {
        printf("load_spam failed\n");
        return -1;
    }

    if (load_random() != 0) {
        printf("load_random failed\n");
        return -1;
    }

    if (load_access() != 0) {
        printf("load_access failed\n");
        return -1;
    }

    if (load_pr() != 0) {
        printf("load_pr failed\n");
        return -1;
    }

    return 0;
}

int main (int argc, char** argv) {

    if (load_all() !=0) {
        printf("load_all failed\n");
        exit(1);
    }

    sort_all();

    if (write_all() != 0) {
        printf("write_all failed\n");
        exit(1);
    }

    printf("Happy Ending!\n");

    return 0;
}
