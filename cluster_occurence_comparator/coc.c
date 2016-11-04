#include "coc.h"

int getClusterId(int clustering, int doc_id) {
    int cluster_id = -1;

    if (clustering == 1) {
        cluster_id = doc_to_cluster_map_for_c1[doc_id-1];
    } else if (clustering == 2) {
        cluster_id = doc_to_cluster_map_for_c2[doc_id-1];
    }

    return cluster_id;
}

void getIncludedDocsFromInvertedIndex (int count, int *docs) {
    Posting *postings = (Posting *) malloc(sizeof(Posting) * count);

    /* fseeko(inverted_index, address, SEEK_SET); */
    fread(postings, POSTING_SIZE, count, inverted_index);
    for (int i = 0; i < count; i++) {
        docs[i] = postings[i].doc_id;
    }

    free(postings);
}

unsigned int internal_comparison (long *ttc, int term_index) {
    unsigned int term_posting_count = terms[term_index].total_count;
    unsigned int table_pointer_alloc_size = config->number_of_clusters_for_c1 * sizeof(bool *);
    unsigned int table_alloc_size = config->number_of_clusters_for_c2 * sizeof(bool);
    int *included_doc_ids = malloc(sizeof(int) * term_posting_count);
    bool **c1_vs_c2_occurence_table = malloc (table_pointer_alloc_size);
    /* long address = (*ttc) * POSTING_SIZE; */
    unsigned int total = 0;

    for (int i = 0; i < config->number_of_clusters_for_c1; i++) {
        c1_vs_c2_occurence_table[i] = malloc(table_alloc_size);
    }

    for (int i = 0; i < config->number_of_clusters_for_c1; i++) {
        for (int j = 0; j < config->number_of_clusters_for_c2; j++) {
            c1_vs_c2_occurence_table[i][j] = false;
        }
    }

    getIncludedDocsFromInvertedIndex(term_posting_count, included_doc_ids);
    for (int i = 0; i < term_posting_count; i++) {
        int cluster_id_for_c1 = getClusterId(1, included_doc_ids[i]);
        int cluster_id_for_c2 = getClusterId(2, included_doc_ids[i]);
        c1_vs_c2_occurence_table[cluster_id_for_c1][cluster_id_for_c2] = true;
    }

    for (int i = 0; i < config->number_of_clusters_for_c1; i++) {
        for (int j = 0; j < config->number_of_clusters_for_c2; j++) {
            if (c1_vs_c2_occurence_table[i][j] == true) {
                total++;
            }
        }
    }

    (*ttc) += terms[term_index].total_count;
    free(included_doc_ids);
    for (int i = 0; i < config->number_of_clusters_for_c1; i++) {
        free(c1_vs_c2_occurence_table[i]);
    }
    free(c1_vs_c2_occurence_table);

    return total;
}

bool readDocToClusterMap (int clustering) {
    char line[256];
    char *c;
    int index, doc_id, cluster_id;
    FILE *file;
    int *array;

    if (clustering == 1) {
        file = doc_to_cluster_map_for_c1_file;
        array = doc_to_cluster_map_for_c1;
    } else if (clustering == 2) {
        file = doc_to_cluster_map_for_c2_file;
        array = doc_to_cluster_map_for_c2;
    }

    index = 0;
    while (fgets(line, sizeof(line), file)) {
        c = strtok(line, " ");
        sscanf(c, "%d", &doc_id);
        c = strtok(NULL, " ");
        sscanf(c, "%d", &cluster_id);

        if (index != doc_id-1)
            return false;
        array[doc_id-1] = cluster_id;
        index++;
    }

    return true;
}

bool readDocToClusterMaps () {
    if (readDocToClusterMap(1) && readDocToClusterMap(2)) {
        return true;
    }

    return false;
}

void writeResults () {
    FILE *fp;
    long total = 0;

    if (!(fp = fopen(COC_RESULT_FILENAME, "w"))) {
        state = COULD_NOT_OPEN_COC_RESULT_FILE;
        return;
    }

    for (int i = 0; i < config->number_of_terms; i++) {
        fprintf(fp, "%d %d\n", i + 1, terms_coc_counts[i]);
        total += terms_coc_counts[i];
    }
    fprintf(fp, "Total: %ld", total);

    fclose(fp);
    state = SUCCESS;
}

void compare () {
    long total_term_occurences = 0;
    int count_M = 0;

    if (!readDocToClusterMaps()) {
        state = COULD_NOT_READ_DOC_TO_CLUSTER_MAP;
        return;
    }

    for (int i = 0; i < config->number_of_terms; i++) {
        terms_coc_counts[i] = internal_comparison(&total_term_occurences, i);
        if (terms_coc_counts[i] == 0) {
            state = WARNING_AT_CLUSTER_OCCURENCE_COMPARE;
            return;
        }

        if (i % 1000000 == 0) {
            printf("Comparison for term %dM done.\n", count_M++);
        }
    }

    state = SUCCESS;
}

int initCOC (Conf *conf) {
    if (!conf) {
        state = EMPTY_CONFIG_DATA;
        return -1;
    }

    config = conf;

    long term_alloc_size = config->number_of_terms * sizeof(Term);

    if (!(terms = malloc(term_alloc_size))) {
        state = COULD_NOT_ALLOCATE_TERMS;
        return -1;
    }

    long terms_coc_counts_size = config->number_of_terms * sizeof(int);
    if (!(terms_coc_counts = malloc(terms_coc_counts_size))) {
        state = COULD_NOT_ALLOCATE_TERMS_COC_COUNTS;
        return -1;
    }

    long doc_to_cluster_map_size = config->number_of_documents * sizeof(int);
    if (!(doc_to_cluster_map_for_c1 = malloc(doc_to_cluster_map_size))) {
        state = COULD_NOT_ALLOCATE_DOC_TO_CLUSTER_MAP;
        return -1;
    }

    if (!(doc_to_cluster_map_for_c2 = malloc(doc_to_cluster_map_size))) {
        state = COULD_NOT_ALLOCATE_DOC_TO_CLUSTER_MAP;
        return -1;
    }

    if (!(inverted_index = fopen(config->inverted_index_path, "r"))) {
        state = COULD_NOT_OPEN_INVERTED_INDEX;
        return -1;
    }

    if (!(doc_to_cluster_map_for_c1_file = fopen(config->doc_to_cluster_map_path_for_c1, "r"))) {
        state = COULD_NOT_OPEN_DOC_TO_CLUSTER_MAP_FOR_C1;
        return -1;
    }

    if (!(doc_to_cluster_map_for_c2_file = fopen(config->doc_to_cluster_map_path_for_c2, "r"))) {
        state = COULD_NOT_OPEN_DOC_TO_CLUSTER_MAP_FOR_C2;
        return -1;
    }

    state = SUCCESS;
    return 0;
}
