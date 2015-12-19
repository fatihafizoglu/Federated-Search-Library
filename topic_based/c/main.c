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


    unsigned int sample_count = conf.number_of_documents * PERCENTAGE_OF_SAMPLES;
    unsigned int sample_indeces[sample_count];
    randomSample(&sample_indeces, sample_count, config->number_of_terms);

    for (size_t i = 0; i < sample_count; i++) {
        if (sample_indeces[i] >= 10000 && sample_indeces[i] <= 20000)
            printf("%d-", sample_indeces[i]);
    }

    //openDocumentVectorsFiles();
    ///actState();
    //loadDocuments();
    //actState();

    //for (size_t i = 14999999; i < 20000000; i++) {
        //TermVectors temp = getTermVectors(&documents[i]);
        /*
        for (size_t j = 0; j < documents[i].uterm_count; j++) {
            printf("%d ", temp[j].term_id);
        }
        printf("\n");*/
        //free(temp);
    //}


    //closeDocumentVectorsFiles();
    //actState();



    printf("sleep\n");
    sleep(100);
    return 0;
}
