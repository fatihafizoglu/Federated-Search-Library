#include "diversify.h"
#include <unistd.h>

int main (int argc, char *argv[]) {
    char wordlist_path[FILEPATH_LENGTH] = "/home/eckucukoglu/projects/ms-thesis/allocation_runs/topic_based_2/csi_wordlist_idf.txt";
    char document_info_path[FILEPATH_LENGTH] = "/home/eckucukoglu/projects/ms-thesis/main_index/doc_lengths.txt";
    char document_vectors_folder_path[FILEPATH_LENGTH] = "/home/eckucukoglu/projects/ms-thesis/main_index/doc_vectors";
    char preresults_path[FILEPATH_LENGTH] = "/home/eckucukoglu/projects/ms-thesis/results/1.txt";
    unsigned int number_of_documents = 50220538;
    unsigned int number_of_terms = 163629158;
    unsigned int number_of_preresults = 1000;
    unsigned int number_of_query = 50;
    unsigned int diversification_algorithm = MAX_SUM;

    Conf conf = {
        .wordlist_path = wordlist_path,
        .document_info_path = document_info_path,
        .document_vectors_folder_path = document_vectors_folder_path,
        .preresults_path = preresults_path,
        .number_of_documents = number_of_documents,
        .number_of_terms = number_of_terms,
        .number_of_preresults = number_of_preresults,
        .number_of_query = number_of_query,
        .diversification_algorithm = diversification_algorithm
    };

    {
        initDiversify(&conf); // TODO: free allocated areas.
        // loadDocuments();
        // loadTerms();
        loadPreresults();
        diversify();
        writeResults();
        // openDocumentVectorsFiles();
        // actState();
        // openClusterDocumentIdsFiles();
        // actState();
        // initClusters();
        // actState();
        // sampleDocuments();
        // actState();
        //
        // initializeKMeans();
        // swapDictionary();
        // printf("initializeKMeans end.\n");
        // for (i = 0; i < 5; i++) {
        //     kMeans();
        //     swapDictionary();
        //     printf("kMeans %d end.\n", i+1);
        // }
        // assignDocumentsToClusters();
        // printf("assignDocumentsToClusters end.\n");
        // endProgram ();
    }

    return 0;
}
