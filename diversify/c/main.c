#include "diversify.h"
#include <unistd.h>

int main (int argc, char *argv[]) {
    char wordlist_path[FILEPATH_LENGTH] = "/home1/grupef/ecank/data/wordlist_IDF";
    char document_info_path[FILEPATH_LENGTH] = "/home1/grupef/ecank/data/doc_lengths";
    char document_vectors_folder_path[FILEPATH_LENGTH] = "/home1/grupef/ecank/data/document_vectors";
    char preresults_path[FILEPATH_LENGTH] = "";
    unsigned int number_of_documents = 50220538;
    unsigned int number_of_terms = 163629158;
    unsigned int number_of_preresults = 200;
    unsigned int number_of_results = 20;
    unsigned int number_of_query = 198;
    unsigned int div_algorithms[] = {MAX_SUM, SF};
    double div_lambdas[] = { 0.25, 0.5, 0.75, 1 };

    int i, j;
    int div_len = sizeof(div_algorithms) / sizeof(unsigned int);
    int lambda_len = sizeof(div_lambdas) / sizeof(double);

    /* It is convenient to gather some information from command line. */
    strcpy(preresults_path, argv[1]);
    strcpy(wordlist_path, argv[2]);
    if (argc > 3) {
        sscanf(argv[3], "%d", &number_of_results);
    }

#ifdef DEBUG
    prinf("Wordlist: %s\n", wordlist_path);
    prinf("Preresults: %s\n", preresults_path);
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
        .number_of_query = number_of_query
    };

    for (i = 0; i < div_len; i++) {
        for (j = 0; j < lambda_len; j++) {
            conf.diversification_algorithm = div_algorithms[i];
            conf.lambda = div_lambdas[j];

            initDiversify(&conf);
            openDocumentVectorsFiles();
            loadDocuments();
            loadTerms();
            loadPreresults();
            diversify();
            writeResults();
            endProgram ();
        }
    }

    return 0;
}
