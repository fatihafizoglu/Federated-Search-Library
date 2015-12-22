#include "Allocator.h"
#include <unistd.h>

int main (int argc, char *argv[]) {

    char wordlist_path[FILEPATH_LENGTH] = "/media/eckucukoglu/1000/Dropbox/projects/ms-thesis/merged_wordlist.txt";
    char document_info_path[FILEPATH_LENGTH] = "/media/eckucukoglu/1000/Dropbox/projects/ms-thesis/merged_SMART-documents.txt";
    char document_vectors_folder_path[FILEPATH_LENGTH] = "/media/eckucukoglu/1000/Dropbox/projects/ms-thesis";
    unsigned int number_of_documents = 50220538;
    unsigned int number_of_terms = 163629158;

    Conf conf = {
        .wordlist_path = wordlist_path,
        .document_info_path = document_info_path,
        .document_vectors_folder_path = document_vectors_folder_path,
        .number_of_documents = number_of_documents,
        .number_of_terms = number_of_terms
    };

    initAllocator(&conf);
    actState();

    openClusterDocumentIdsFiles();
    actState();

    writeDocumentIdToClusterFile(0, 50000212);
    actState();

    writeDocumentIdToClusterFile(1, 50000215);
    actState();
    writeDocumentIdToClusterFile(1, 40000);
    actState();

    closeClusterDocumentIdsFiles();
    actState();
    /*
    openDocumentVectorsFiles();
    actState();

    loadDocuments();
    actState();

    initClusters();
    actState();
    sampleDocuments();
    actState();

    initializeKMeans();
    swapDictionary();
    actState();

    for (int i = 0; i < 5; i++) {
        kMeans();
        swapDictionary();
        actState();
        printf(">> kmeans(%d) finished\n", i);
    }

    closeDocumentVectorsFiles();
    */


    // Test getDocument, first loadDocuments needed.
    /*Document *doci = getDocument(4);
    printf("%d\n", doci->doc_id);*/

    // Test sample document ids
    /*for (size_t i = 0; i < 502205; i++) {
        //if (sample_doc_ids[i] >= 10000 && sample_doc_ids[i] <= 20000)
        printf("%d-", sample_doc_ids[i]);
    }*/

    // Test getTermVectors
    /*for (size_t i = 14999999; i < 20000000; i++) {
        TermVectors temp = getTermVectors(&documents[i]);

        for (size_t j = 0; j < documents[i].uterm_count; j++) {
            printf("%d ", temp[j].term_id);
        }
        printf("\n");
        free(temp);
    }*/


    //closeDocumentVectorsFiles();
    //actState();



    printf("sleep\n");
    sleep(3);
    return 0;
}
