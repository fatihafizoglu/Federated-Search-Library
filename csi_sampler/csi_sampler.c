#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <time.h>
#include <math.h>
#include <sys/time.h>

/*******************/
/***** CONFIGS *****/
/*******************/
#define ACCESS_INFO "/home1/grupef/ecank/data/CSI/desc_access_counts"
#define PR_INFO "/home1/grupef/ecank/data/CSI/doc-PR-map2019.txt_PRsorted"
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
unsigned int random_docs[NOF_SAMPLED_DOCS];
unsigned int access_docs[NOF_SAMPLED_DOCS];
unsigned int rp_docs[NOF_SAMPLED_DOCS];

/*********************/
/***** FUNCTIONS *****/
/*********************/
int write(char *output_path, unsigned int *docs) {
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
    qsort(access_docs, NOF_SAMPLED_DOCS, sizeof(unsigned int), cmpfunc);
    qsort(rp_docs, NOF_SAMPLED_DOCS, sizeof(unsigned int), cmpfunc);
}

/* Input file format: (pr desc-sorted)
 * <doc_id> <pr>
 * 42722787 1080.163861
 */
int load_pr () {
    unsigned int doc_id;
    double pagerank;
    unsigned int count = 0;
    FILE *fp = fopen(PR_INFO, "r");

    if (fp == NULL) {
        printf("%s fopen failed\n", PR_INFO);
        return -1;
    }

    while ((count < NOF_SAMPLED_DOCS) && !feof(fp)) {
        fscanf (fp, "%u %lf\n", &(doc_id), &(pagerank));

        if (spam_scores[doc_id] < SPAM_THRESHOLD)
            continue;

        rp_docs[count++] = doc_id;
    }

    printf("%s: count:%u\n", __FUNCTION__, count);

    fclose(fp);
    return 0;
}

/* Input file format: (count desc-sorted)
 * <count> <doc_id> <doc_name>
 * 57207 42710475 clueweb09-en0011-58-19250
 */
int load_access () {
    unsigned int doc_id;
    char doc_name[255];
    unsigned int occ_count;
    unsigned int count = 0;
    FILE *fp = fopen(ACCESS_INFO, "r");

    if (fp == NULL) {
        printf("%s fopen failed\n", ACCESS_INFO);
        return -1;
    }

    while ((count < NOF_SAMPLED_DOCS) && !feof(fp)) {
        fscanf (fp, "%u %u %s\n", &(occ_count), &(doc_id), doc_name);

        if (spam_scores[doc_id] < SPAM_THRESHOLD)
            continue;

        access_docs[count++] = doc_id;
    }

    printf("%s: count:%u\n", __FUNCTION__, count);

    fclose(fp);
    return 0;
}

// Assumes 0 <= max <= RAND_MAX
// Returns in the closed interval [0, max]
long random_at_most(long max) {
    unsigned long
        // max <= RAND_MAX < ULONG_MAX, so this is okay.
        num_bins = (unsigned long) max + 1,
        num_rand = (unsigned long) RAND_MAX + 1,
        bin_size = num_rand / num_bins,
        defect   = num_rand % num_bins;

    long x;
    do {
        x = random();
    }
    // This is carefully written not to overflow
    while (num_rand - defect <= (unsigned long)x);

    // Truncated division is intentional
    return x/bin_size;
}

int load_random () {
    unsigned int count = 0;
    char *marks;

    marks = malloc(NOF_DOCS * sizeof(char));

    struct timeval tm;
    gettimeofday(&tm, NULL);
    srandom(tm.tv_sec + tm.tv_usec * 1000000ul);

    marks[0] = '.';
    while (count < NOF_SAMPLED_DOCS) {

        long number = random_at_most(NOF_DOCS-1);
        if (marks[number] == '.') {
            continue;
        }

        if (spam_scores[number] < SPAM_THRESHOLD)
            continue;

        random_docs[count++] = number;
        marks[number] = '.';
    }

    printf("%s: count:%u\n", __FUNCTION__, count);

    free(marks);
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
