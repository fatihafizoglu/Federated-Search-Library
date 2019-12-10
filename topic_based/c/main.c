#include "Allocator.h"
#include <unistd.h>

int main (int argc, char *argv[]) {
    int i;
    char wordlist_path[FILEPATH_LENGTH] = "/home1/grupef/ecank/data/wordlist";
    char document_lengths[FILEPATH_LENGTH] = "/home1/grupef/ecank/data/doc_lengths";
    char document_vectors_folder_path[FILEPATH_LENGTH] = "/home1/grupef/ecank/data/document_vectors";
    unsigned int number_of_documents = 50220538;
    unsigned int number_of_terms = 163629158;

    Conf conf = {
        .wordlist_path = wordlist_path,
        .document_info_path = document_lengths,
        .document_vectors_folder_path = document_vectors_folder_path,
        .number_of_documents = number_of_documents,
        .number_of_terms = number_of_terms,

        .DIVERSIFY = false
    };

    {
        initAllocator(&conf);
        actState();
        loadDocuments();
        actState();
        openDocumentVectorsFiles();
        actState();
        openClusterDocumentIdsFiles();
        actState();
        initClusters();
        actState();
        sampleDocuments();
        actState();

        initializeKMeans();
        swapDictionary();
        printf("initializeKMeans end.\n");
        for (i = 0; i < 5; i++) {
            kMeans();
            swapDictionary();
            printf("kMeans %d end.\n", i+1);
        }
        assignDocumentsToClusters();
        printf("assignDocumentsToClusters end.\n");
        endProgram ();
    }

    return 0;
}
