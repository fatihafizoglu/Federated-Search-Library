#include "coc.h"
#include <unistd.h>

int main (int argc, char *argv[]) {
    /* Files */
    char inverted_index_path[FILEPATH_LENGTH] = "/home/eckucukoglu/msData/inverted_index";
    char wordlist_path[FILEPATH_LENGTH] = "/media/eckucukoglu/solar/ms-thesis/main_index/wordlist_idf.txt";
    char doc_to_cluster_map_path_for_c1[FILEPATH_LENGTH] = "/media/eckucukoglu/solar/ms-thesis/allocation_runs/random_based_1/doc-to-cluster-map_text/doc_sorted";
    char doc_to_cluster_map_path_for_c2[FILEPATH_LENGTH] = "/media/eckucukoglu/solar/ms-thesis/allocation_runs/topic_based_2/doc-to-cluster-map_text/doc_sorted";
    /* Variables */
    unsigned int number_of_terms = 163629158;
    unsigned int number_of_documents = 50220538;
    unsigned int number_of_clusters_for_c1 = 100;
    unsigned int number_of_cluster_for_c2 = 100;

    Conf conf = {
        .inverted_index_path = inverted_index_path,
        .wordlist_path = wordlist_path,
        .doc_to_cluster_map_path_for_c1 = doc_to_cluster_map_path_for_c1,
        .doc_to_cluster_map_path_for_c2 = doc_to_cluster_map_path_for_c2,

        .number_of_terms = number_of_terms,
        .number_of_documents = number_of_documents,
        .number_of_clusters_for_c1 = number_of_clusters_for_c1,
        .number_of_clusters_for_c2 = number_of_cluster_for_c2
    };

    {
        initCOC(&conf);
        actState();
        loadTerms();
        actState();
        compare();
        actState();
        writeResults();
        actState();
        endProgram();
        actState();
    }

    return 0;
}
