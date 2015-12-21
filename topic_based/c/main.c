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
    //loadDocuments();
    //actState();

    //initClusters(PERCENTAGE_OF_SAMPLES, NUMBER_OF_CLUSTERS);
    //openDocumentVectorsFiles();
    //actState();

    // Test dictionary dynamic hash table
    Dict d = DictCreate();
    DictInsert(d, 2, 5);
    printf("%d\n", DictSearch(d, 2));

    for(int i = 0; i < 10000; i++) {
        DictInsert(d, i, i+1);
    }

    DictInsertOrUpdate(d, 2, 100);
    printf("%d\n", DictSearch(d, 2));

    DictInsertOrUpdate(d, 163000000, 100);
    printf("%d\n", DictSearch(d, 163000000));

    DictInsertOrUpdate(d, 163000000, 50);
    printf("%d\n", DictSearch(d, 163000000));

    DictDestroy(d);

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
    sleep(100);
    return 0;
}
