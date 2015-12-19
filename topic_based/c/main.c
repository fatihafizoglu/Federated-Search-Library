#include "Allocator.h"
#include <unistd.h>

int main (int argc, char *argv[]) {

    char wordlist_path[FILEPATH_LENGTH] = "/media/eckucukoglu/1000/Dropbox/projects/ms-thesis/merged_wordlist.txt";
    char document_info_path[FILEPATH_LENGTH] = "/media/eckucukoglu/1000/Dropbox/projects/ms-thesis/merged_SMART-documents.txt";
    char document_vectors_folder_path[FILEPATH_LENGTH] = "/media/eckucukoglu/1000/Dropbox/projects/ms-thesis";
    int number_of_documents = 50220538;
    int number_of_terms = 163629158;

    Conf conf = {
        .wordlist_path = wordlist_path,
        .document_info_path = document_info_path,
        .document_vectors_folder_path = document_vectors_folder_path,
        .number_of_documents = number_of_documents,
        .number_of_terms = number_of_terms
    };

    initAllocator(&conf);
    actState();
    //int c = loadTerms();

    printf("<%s>\n", getDocumentVectorsFilepath(50000000));

    int c = loadDocuments();
    printf("%d\n", c);
    actState();

    printf("sleep\n");
    sleep(100);
    return 0;
}
