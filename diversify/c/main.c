#include "diversify.h"
#include <unistd.h>
#include <time.h>

int main (int argc, char *argv[]) {
    double time_spent = 0.0;
    clock_t begin = clock();
    char wordlist_path[FILEPATH_LENGTH] = "/home1/grupef/ecank/data/wordlist_IDF"; // DONTCHANGE
    char document_info_path[FILEPATH_LENGTH] = "/home1/grupef/ecank/data/doc_lengths"; // DONTCHANGE
    char document_vectors_folder_path[FILEPATH_LENGTH] = "/home1/grupef/ecank/data/document_vectors"; // DONTCHANGE
    char preresults_path[FILEPATH_LENGTH] = ""; // CHANGE -> GET FROM ARGUMENTS
    unsigned int number_of_documents = 50220538; // DONTCHANGE
    unsigned int number_of_terms = 163629158; // DONTCHANGE
    unsigned int number_of_preresults = 0;//100; // CHANGE -> GET FROM ARGUMENTS
    unsigned int number_of_results = 0;//20; // CHANGE -> GET FROM ARGUMENTS
    unsigned int number_of_query = 0;//198; // CHANGE -> GET FROM ARGUMENTS
    unsigned int div_algorithms[] = {MAX_SUM, SY}; // DONTCHANGE
    double div_lambdas[] = { 0.25, 0.5, 0.75 }; // DONTCHANGE

    int i, j;
    int div_len = sizeof(div_algorithms) / sizeof(unsigned int);
    int lambda_len = sizeof(div_lambdas) / sizeof(double);

    /* It is convenient to gather some information from command line. */
    strcpy(preresults_path, argv[1]);
    sscanf(argv[2], "%d", &number_of_preresults);
    sscanf(argv[3], "%d", &number_of_results);
    sscanf(argv[4], "%d", &number_of_query);

#ifdef DEBUG
    printf("Wordlist: %s\n", wordlist_path);
    printf("Preresults: %s\n", preresults_path);
    printf("Document info path (doc lengths): %s\n", document_info_path);
    printf("Document vectors folder: %s\n", document_vectors_folder_path);
    printf("Number of preresults: %d\n", number_of_preresults);
    printf("Number of results: %d\n", number_of_results);
    printf("Number of query: %d\n", number_of_query);

    printf("Now, you have 10 seconds to send SIGINT...\n");
    fflush(stdout);
    sleep(10);
#endif

    Conf conf = {
        .wordlist_path = wordlist_path,
        .document_info_path = document_info_path,
        .document_vectors_folder_path = document_vectors_folder_path,
        .preresults_path = preresults_path,
        .number_of_documents = number_of_documents,
        .number_of_terms = number_of_terms,

        .DIVERSIFY = true,
        .number_of_preresults = number_of_preresults,
        .number_of_results = number_of_results,
        .number_of_query = number_of_query,
        .real_number_of_query = 0
    };

    initDiversify(&conf);
    openDocumentVectorsFiles();
    loadDocuments();
    loadTerms();
    loadPreresults();

#ifdef DEBUG
    printf("LOADING DONE\n");
    fflush(stdout);
#endif

    for (i = 0; i < div_len; i++) {
        for (j = 0; j < lambda_len; j++) {
            conf.diversification_algorithm = div_algorithms[i];
            conf.lambda = div_lambdas[j];

            cleanResults();
            diversify();
            writeResults();
        }
    }
    endProgram();

    clock_t end = clock();
    time_spent += (double)(end - begin) / CLOCKS_PER_SEC;
    printf("time %f seconds\n", time_spent);

    return 0;
}
