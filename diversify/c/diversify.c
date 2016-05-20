#include "diversify.h"

void writeResults () {
    FILE *fp;
    char results_path[FILEPATH_LENGTH] = "";
    size_t len1 = strlen(config->preresults_path);
    memcpy(results_path, config->preresults_path, len1);
    memcpy(results_path+len1, "_diversified", 12);

    if (!(fp = fopen(results_path, "w"))) {
        return;
    }

    for (int q_no = 0; q_no < config->number_of_query; q_no++)
        for (int j = 0; j < config->number_of_preresults; j++)
            if (results[q_no][j].doc_id != 0 && results[q_no][j].score != 0)
                fprintf(fp, "%d\tQ0\t%d\t%d\t%lf\tfs\n", q_no + 1, results[q_no][j].doc_id, j + 1, results[q_no][j].score);

    fclose(fp);
}

void loadPreresults () {
    FILE *fp;
    char temp[100];
    unsigned int query_id;
    unsigned int document_id;
    unsigned int rank;
    double score;

    if (!(fp = fopen(config->preresults_path, "r"))) {
        return;
    }

    while (!feof(fp)) {
        fscanf (fp, "%u %s %u %u %lf %s\n", &(query_id), temp, &(document_id),
                                           &(rank), &(score), temp);
        preresults[query_id-1][rank-1].doc_id = document_id;
        preresults[query_id-1][rank-1].score = score;
    }

    fclose(fp);
}

int initDiversify (Conf *conf) {
    if (!conf) {
        state = EMPTY_CONFIG_DATA;
        return -1;
    }

    config = conf;

    long term_alloc_size = config->number_of_terms * sizeof(Term);
    long documents_alloc_size = config->number_of_documents * sizeof(Document);
    long results_pointer_alloc_size = config->number_of_query * sizeof(Result *);
    long results_alloc_size = config->number_of_preresults * sizeof(Result);

    if (!(terms = malloc(term_alloc_size))) {
        state = COULD_NOT_ALLOCATE_TERMS;
        return -1;
    }

    if (!(documents = malloc(documents_alloc_size))) {
        state = COULD_NOT_ALLOCATE_DOCUMENTS;
        return -1;
    }

    if (!(preresults = malloc(results_pointer_alloc_size))) {
        return -1;
    }

    for (int i = 0; i < config->number_of_query; i++) {
        if (!(preresults[i] = malloc(results_alloc_size))) {
            return -1;
        }
    }

    if (!(results = malloc(results_pointer_alloc_size))) {
        return -1;
    }

    for (int i = 0; i < config->number_of_query; i++) {
        if (!(results[i] = malloc(results_alloc_size))) {
            return -1;
        }
    }

    for (int i = 0; i < config->number_of_query; i++) {
        for (int j = 0; j < config->number_of_preresults; j++) {
            preresults[i][j].doc_id = 0;
            preresults[i][j].score = 0;
            results[i][j].doc_id = 0;
            results[i][j].score = 0;
        }
    }

    return 0;
}

int main (int argc, char *argv[]) {
    char wordlist_path[FILEPATH_LENGTH] = "/home/eckucukoglu/projects/ms-thesis/allocation_runs/topic_based_2/csi_wordlist_idf.txt";
    char document_info_path[FILEPATH_LENGTH] = "/home/eckucukoglu/projects/ms-thesis/main_index/doc_lengths.txt";
    char document_vectors_folder_path[FILEPATH_LENGTH] = "/home/eckucukoglu/projects/ms-thesis/main_index/doc_vectors";
    char preresults_path[FILEPATH_LENGTH] = "/home/eckucukoglu/projects/ms-thesis/results/1.txt";
    unsigned int number_of_documents = 50220538;
    unsigned int number_of_terms = 163629158;
    unsigned int number_of_preresults = 1000;
    unsigned int number_of_query = 50;

    Conf conf = {
        .wordlist_path = wordlist_path,
        .document_info_path = document_info_path,
        .document_vectors_folder_path = document_vectors_folder_path,
        .preresults_path = preresults_path,
        .number_of_documents = number_of_documents,
        .number_of_terms = number_of_terms,
        .number_of_preresults = number_of_preresults,
        .number_of_query = number_of_query
    };

    {
        initDiversify(&conf); // TODO: free allocated areas.
        // loadDocuments();
        // loadTerms();
        loadPreresults();


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
