#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define DOC_ID_TERM_FREQUENCY_PAIR_SIZE_IN_BYTES 8
#define DOC_NO 50220539
#define NO_OF_QUERY 50
#define NO_OF_CLUSTERS 100
#define QUERY_FILE_PATH "2009_queries_MAIN.txt"
#define SELECTED_RESOURCES_FILE_PATH "2009_selected_resources_TOPIC_CSI.txt"
#define TOPIC_DOCUMENT_CLUSTER_MAP_FILE_PATH "/media/fatihafizoglu/LenovoMS/MS/TopicBasedClusters_100_2/read/final/cluster_concat_sorted"
#define RANDOM_DOCUMENT_CLUSTER_MAP_FILE_PATH "/media/fatihafizoglu/LenovoMS/MS/RandomBasedClusters_100/read/final/cluster_concat_sorted"
#define INDEX_FILE_PATH "/media/fatihafizoglu/LenovoMS/MS/Index/merged_entry.txt"
#define NO_OF_METHOD 4 //EXHAUSTIVE, TOPIC, TOPIC_AND_RANDOM, RANDOM

typedef struct Term {
    unsigned int occurange_in_query;
    unsigned int occurange_in_docs;
    double cfc_weight;
    long unsigned disk_address;
} term;

typedef struct Query {
    unsigned int queryLength;
    term * terms;
} query;

query *queries;
unsigned int **selected_resources;
unsigned int *selected_resources_lengths;
unsigned int *topic_document_cluster_map;
unsigned int *random_document_cluster_map;

void print_time(char *explanation) {
    time_t timer;
    char buffer[26];
    struct tm* tm_info;

    time(&timer);
    tm_info = localtime(&timer);

    strftime(buffer, 26, "%H:%M:%S %m/%d/%y", tm_info);
    printf("%s %s\n", explanation, buffer);
}

void read_queries() {
    FILE *fp;
    unsigned int query_index = 0;
    unsigned int i;

    fp = fopen(QUERY_FILE_PATH, "r");
    if (!fp) {
        printf("ERROR: Query file not found: %s", QUERY_FILE_PATH);
        exit(1);
    }

    while (fscanf(fp, "%u", &queries[query_index].queryLength) != EOF) {
        
        queries[query_index].terms = malloc(queries[query_index].queryLength * sizeof(term));

        for (i = 0; i < queries[query_index].queryLength; i++) {
            fscanf(fp,"%u", &queries[query_index].terms[i].occurange_in_query);
            fscanf(fp,"%u %lf %lu", &queries[query_index].terms[i].occurange_in_docs, &queries[query_index].terms[i].cfc_weight, &queries[query_index].terms[i].disk_address);
        }

        query_index += 1;
    }
    fclose(fp);
}

void read_selected_clusters() {
    FILE *fp;
    unsigned int query_index = 0;
    unsigned int i;
    char temp;

    fp = fopen(SELECTED_RESOURCES_FILE_PATH, "r");
    if (!fp) {
        printf("ERROR: Selected resources file not found: %s", SELECTED_RESOURCES_FILE_PATH);
        exit(1);
    }

    while (fscanf(fp, "%u", &selected_resources_lengths[query_index]) != EOF) {

        selected_resources[query_index] = (unsigned int*) malloc(selected_resources_lengths[query_index] * sizeof(unsigned int));
        
        if (selected_resources_lengths[query_index] > 0) {    
            i = 0;
            while (1) {
                fscanf(fp, "%u%c", &selected_resources[query_index][i], &temp);
                if (temp == '\n')
                    break;

                i += 1;
            }
        }

        query_index += 1;
    }
    fclose(fp);
}

void read_cluster_map(unsigned int *map, char *map_path) {
    FILE *fp;
    unsigned int document_id, temp_document_id;

    fp = fopen(map_path, "r");
    if (!fp) {
        printf("ERROR: Cluster map file not found: %s", map_path);
        exit(1);
    }

    document_id = 1;
    while (fscanf(fp, "%u %u", &temp_document_id, &map[document_id]) != EOF) {
        if (document_id != temp_document_id) {
            printf("ERROR: How is that even possible?!?");
            exit(1);
        }

        document_id += 1;
    }
    fclose(fp);
}

void main(void) {
    
    FILE *fp;
    FILE **csvs;
    unsigned int query_index, i, j, k;
    unsigned int total_posting_list_checked;
    unsigned int max_posting_list_checked;
    unsigned int *posting_list;
    unsigned int *document_accesses;
    unsigned int topic_cluster_id;
    unsigned int random_cluster_id;


    // Variables that keeps statistics
    unsigned int ***cluster_accesses;
    unsigned int **total_posting_lists;
    unsigned int **max_posting_lists;

    print_time("Script Started at:");

    queries = (query*) malloc(NO_OF_QUERY * sizeof(query));
    selected_resources = (unsigned int**) malloc(NO_OF_QUERY * sizeof(unsigned int*));
    selected_resources_lengths = (unsigned int*) malloc(NO_OF_QUERY * sizeof(unsigned int));
    topic_document_cluster_map = (unsigned int*) malloc(DOC_NO * sizeof(unsigned int));
    random_document_cluster_map = (unsigned int*) malloc(DOC_NO * sizeof(unsigned int));

    read_queries();
    read_selected_clusters();
    read_cluster_map(topic_document_cluster_map, TOPIC_DOCUMENT_CLUSTER_MAP_FILE_PATH);
    read_cluster_map(random_document_cluster_map, RANDOM_DOCUMENT_CLUSTER_MAP_FILE_PATH);

    // Init variables that keeps statistics
    csvs = (FILE**) malloc(NO_OF_METHOD * sizeof(FILE*));

    cluster_accesses = (unsigned int***) malloc(NO_OF_QUERY * sizeof(unsigned int**));
    total_posting_lists = (unsigned int**) malloc(NO_OF_QUERY * sizeof(unsigned int*));
    max_posting_lists = (unsigned int**) malloc(NO_OF_QUERY * sizeof(unsigned int*));
    for (query_index = 0; query_index < NO_OF_QUERY; query_index++) {
        cluster_accesses[query_index] = (unsigned int**) malloc(NO_OF_METHOD * sizeof(unsigned int*));
        total_posting_lists[query_index] = (unsigned int*) malloc(NO_OF_METHOD * sizeof(unsigned int));
        max_posting_lists[query_index] = (unsigned int*) malloc(NO_OF_METHOD * sizeof(unsigned int));
        for (i = 0; i < NO_OF_METHOD; i++) {
            cluster_accesses[query_index][i] = (unsigned int*) malloc(NO_OF_CLUSTERS * sizeof(unsigned int));
            total_posting_lists[query_index][i] = 0;
            max_posting_lists[query_index][i] = 0;
            for (j = 0; j < NO_OF_CLUSTERS; j++) {
                cluster_accesses[query_index][i][j] = 0;
            }
        }
    }

    print_time("Stats Started at:");
    fp = fopen(INDEX_FILE_PATH, "rb");
    for (query_index = 0; query_index < NO_OF_QUERY; query_index++) {
        printf("%u\n", query_index + 1);

        document_accesses = (unsigned int*) malloc(DOC_NO * sizeof(unsigned int));
        for (i = 0; i < DOC_NO; i++) {
            document_accesses[i] = 0;
        }

        for (i = 0; i < queries[query_index].queryLength; i++ ) {
            posting_list = (unsigned int*) malloc(DOC_ID_TERM_FREQUENCY_PAIR_SIZE_IN_BYTES * queries[query_index].terms[i].occurange_in_docs * sizeof(unsigned int));
            fseek(fp, DOC_ID_TERM_FREQUENCY_PAIR_SIZE_IN_BYTES * queries[query_index].terms[i].disk_address, 0);
            fread(posting_list, DOC_ID_TERM_FREQUENCY_PAIR_SIZE_IN_BYTES, queries[query_index].terms[i].occurange_in_docs, fp);

            for (j = 0; j < queries[query_index].terms[i].occurange_in_docs * 2; j += 2 ) {
                document_accesses[posting_list[j]] += 1;
            }
            free(posting_list);
        }

        
        // Calculate statistics new!
        for (i = 0; i < DOC_NO; i++) {
            cluster_accesses[query_index][0][0] += document_accesses[i]; // EXHAUSTIVE - Increment first cluster

            topic_cluster_id = topic_document_cluster_map[i];
            random_cluster_id = random_document_cluster_map[i];

            for (j = 0; j < selected_resources_lengths[query_index]; j++) {
                if (selected_resources[query_index][j] == topic_cluster_id) {
                    cluster_accesses[query_index][1][topic_cluster_id] += document_accesses[i]; // TOPIC - Increment documents topic cluster
                    cluster_accesses[query_index][2][random_cluster_id] += document_accesses[i]; // RANDOM AND TOPIC - Increment documents topic cluster
                    break;
                }
            }

            cluster_accesses[query_index][3][random_cluster_id] += document_accesses[i]; // RANDOM - Increment documents random cluster
        }

        free(document_accesses);
    }
    fclose(fp);

    csvs[0] = fopen("stat_EXHAUSTIVE.csv","w");
    csvs[1] = fopen("stat_TOPIC.csv","w");
    csvs[2] = fopen("stat_RANDOM_AND_TOPIC.csv","w");
    csvs[3] = fopen("stat_RANDOM.csv","w");
    for (query_index = 0; query_index < NO_OF_QUERY; query_index++) {
        for (i = 0; i < NO_OF_METHOD; i++) {
            for (j = 0; j < NO_OF_CLUSTERS; j++) {
                if (j != NO_OF_CLUSTERS - 1)
                    fprintf(csvs[i], "%u,", cluster_accesses[query_index][i][j]);
                else
                    fprintf(csvs[i], "%u\n", cluster_accesses[query_index][i][j]);
                total_posting_lists[query_index][i] += cluster_accesses[query_index][i][j];
                if (cluster_accesses[query_index][i][j] > max_posting_lists[query_index][i])
                    max_posting_lists[query_index][i] = cluster_accesses[query_index][i][j];
            }
        }
    }
    fclose(csvs[0]);
    fclose(csvs[1]);
    fclose(csvs[2]);
    fclose(csvs[3]);

    printf("********************************************************************************\n");
    printf("MAX POSTING LIST PROCESSED\n");
    for (query_index = 0; query_index < NO_OF_QUERY; query_index++) {
        for (i = 0; i < NO_OF_METHOD; i++) {
            if (i != NO_OF_METHOD - 1)
                printf("%u,", max_posting_lists[query_index][i]);
            else
                printf("%u\n", max_posting_lists[query_index][i]);
        }
    }
    printf("********************************************************************************\n");
    printf("TOTAL POSTING LIST PROCESSED\n");
    for (query_index = 0; query_index < NO_OF_QUERY; query_index++) {
        for (i = 0; i < NO_OF_METHOD; i++) {
            if (i != NO_OF_METHOD - 1)
                printf("%u,", total_posting_lists[query_index][i]);
            else
                printf("%u\n", total_posting_lists[query_index][i]);
        }
    }

    print_time("Script Ended at:");
    return;
}