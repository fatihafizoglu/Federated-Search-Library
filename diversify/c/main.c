#include "diversify.h"
#include <unistd.h>

int main (int argc, char *argv[]) {
    char wordlist_path[FILEPATH_LENGTH] = "/home/eckucukoglu/projects/ms-thesis/allocation_runs/topic_based_2/csi_wordlist_idf.txt";
    char document_info_path[FILEPATH_LENGTH] = "/home/eckucukoglu/projects/ms-thesis/main_index/doc_lengths.txt";
    char document_vectors_folder_path[FILEPATH_LENGTH] = "/home/eckucukoglu/projects/ms-thesis/main_index/dvec.bins";
    char preresults_path[FILEPATH_LENGTH] = "/home/eckucukoglu/projects/ms-thesis/results/run.txt";
    unsigned int number_of_documents = 50220538;
    unsigned int number_of_terms = 163629158;
    unsigned int number_of_preresults = 200;
    unsigned int number_of_results = 100;
    unsigned int number_of_query = 50;
    unsigned int div_algorithms[] = {MAX_SUM, SF};
    double div_lambdas[] = { 0.25, 0.5, 0.75, 1 };
    
    int i, j;
    int div_len = sizeof(div_algorithms) / sizeof(unsigned int);
    int lambda_len = sizeof(div_lambdas) / sizeof(double);
    
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
            
            {
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
    }
        
    return 0;
}
