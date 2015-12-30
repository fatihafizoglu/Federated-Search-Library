#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include "Allocator.h"

#define DOC_VECTORS_FOLDER "/home/eckucukoglu/Dropbox/projects/ms-thesis"
#define CLUSTERS_TO_DOC_ID_MAPS_FOLDER "/home/eckucukoglu/Dropbox/projects/ms-thesis/"
#define CLUSTER_INDEX_NAME_PREFIX "index_cluster_"

/*
 * Returns filesize of a given filepath.
 */
off_t fsize(const char *filename) {
    struct stat st;
    if (stat(filename, &st) == 0)
        return st.st_size;
    return -1;
}

/*
 * Concatenate given char arrays.
 * Do not forget to free returning object.
 */
char *concatenateStrings (char *first, char *second) {
    char *result;
    size_t len1 = strlen(first);
    size_t len2 = strlen(second);
    result = malloc(len1 + len2 + 1);
    memcpy(result, first, len1);
    memcpy(result+len1, second, len2+1);
    return result;
}

/*
 * For a given cluster id, creates its index file.
 * Basically needs 2 information:
 * - cluster's document id values
 * - collection's document vectors
 */
void createIndex (unsigned int cluster_id) {
    FILE *fp;
    unsigned int *document_ids;
    char cluster_id_str[3];
    char *cluster_document_ids_filename;
    char *cluster_document_ids_filepath;
    unsigned int doc_id;
    unsigned int number_of_documents;
    unsigned int i;

    snprintf(cluster_id_str, sizeof(cluster_id_str), "%d", cluster_id);
    cluster_document_ids_filename =
        concatenateStrings(CLUSTER_DOCUMENT_IDS_VECTOR_FILE_PREFIX, cluster_id_str);
    cluster_document_ids_filepath =
        concatenateStrings(CLUSTERS_TO_DOC_ID_MAPS_FOLDER, cluster_document_ids_filename);
    free(cluster_document_ids_filename);

    if (!(fp = fopen(cluster_document_ids_filepath, "r"))) {
        printf("ERROR: %s could not open.\n", cluster_document_ids_filepath);
        return;
    }

    /* Since each document represented as 4 bytes. */
    number_of_documents = (int)fsize(cluster_document_ids_filepath) / 4;
    if (number_of_documents == -1) {
        printf("ERROR: Could not take size of cluster:%u\n", cluster_id);
        return;
    }

    document_ids = malloc (sizeof(unsigned int) * number_of_documents);
    if (document_ids == NULL) {
        printf("ERROR: Allocation problem (cluster:%u).\n", cluster_id);
        return;
    }

    i = 0;
    while (fread(&doc_id, 1, 4, fp)) {
        document_ids[i] = doc_id;
        i++;
    }

    printf("All document ids read successfully (cluster:%u): %s\n", cluster_id,
        i == number_of_documents ? "YES" : "NO");

    // TODO: use getTermVectors from Allocator.c and write to file.
    // ...

    fclose(fp);
    free(cluster_document_ids_filepath);
    free(document_ids);
}

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
                "\t-c [CLUSTER_ID]\n"
                "\nNote:\n"
                "\tDefine 'cluster to doc id maps folder' and\n"
                "\t'collection documents vectors folder' inside code.\n");
}

int main (int argc, char *argv[]) {
    int check;
    int n = -1, c = -1;
    char *value = NULL;
    unsigned int i;

    if (argc < 3) {
        usage();
        exit(EXIT_SUCCESS);
    }

    opterr = 0;
    while ((check = getopt(argc, argv, "n:c:")) != -1) {
        switch (check) {
        case 'n':
            value = optarg;
            n = atoi(value);
            break;
        case 'c':
            value = optarg;
            c = atoi(optarg);
            break;
        case '?':
            if (optopt == 'c')
                fprintf (stderr, "Option -%c requires a cluster id.\n", optopt);
            else if (optopt == 'n')
                fprintf (stderr, "Option -%c requires a cluster count.\n", optopt);
            else if (isprint (optopt)) {
                fprintf (stderr, "Unknown option `-%c'.\n", optopt);
                usage();
            } else
                fprintf (stderr, "Unknown option character `\\x%x'.\n", optopt);
            return 1;
        default:
            usage();
            return 1;
        }
    }

    for (i = optind; i < argc; i++)
        printf ("Non-option argument %s\n", argv[i]);

    if (n != -1)
        for (i = 0; i < n; i++)
            createIndex(i);

    if (c != -1)
        createIndex(c);

    return 0;
}
