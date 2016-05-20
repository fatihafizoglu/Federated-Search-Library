#include "diversify.h"

double cosineSimilarity (int doc1_id, int doc2_id) {
    double ret = 0.0;
    Document *doc1 = getDocument(doc1_id);
    Document *doc2 = getDocument(doc2_id);
    TermVectors doc1_term_vectors = getTermVectors(doc1);
    TermVectors doc2_term_vectors = getTermVectors(doc2);

    // TODO: calculate cosine similarity

    return ret;
}

void getQueryScores(int q_no, int number_of_results, double *max_score, double *sum_score) {
    int i = 0;

    for (i = 0; i < number_of_results; i++) {
        (*sum_score) += preresults[q_no][i].score;
        if ((*max_score) < preresults[q_no][i].score)
            (*max_score) = preresults[q_no][i].score;
    }
}

void diversifyQuery (int q_no, int algorithm, int number_of_results) {
    if (algorithm == MAX_SUM) {
        int i, j;
        double max_score = 0.0;
        // double sum_score = 0.0;
        double distances[number_of_results][number_of_results];
        double temp;

        memset(distances, 0, number_of_results * number_of_results * sizeof(double));
        // getQueryScores(q_no, number_of_results, &max_score, &sum_score);
        max_score = preresults[q_no][0].score; /* Since rank 1 is the highest score.*/
        for (i = 0; i < number_of_results; i++) {
            for (j = 1; j < number_of_results; j++) {
                temp = preresults[q_no][i].score + preresults[q_no][j].score;
                temp += temp / max_score;
                temp = (1 - MAX_SUM_LAMBDA) * temp;
                distances[i][j] = temp;
                temp = cosineSimilarity(preresults[q_no][i].doc_id, preresults[q_no][j].doc_id);
                temp = 2 * MAX_SUM_LAMBDA * (1 - temp);
                distances[i][j] += temp;
            }
        }

        // TODO: we have work to do at max_sum


    } else if (algorithm == MMR) {
        // TODO: implement MMR

    } else if (algorithm == SY) {
        // TODO: implement SY

    } else {
        /* Let's implement one more algorithm. */
    }
}

int getNumberOfResults (int q_no) {
    int ret = 0;

    for (ret = 0; ret < config->number_of_preresults; ret++) {
        if (preresults[q_no][ret].doc_id == 0 || preresults[q_no][ret].score == 0)
            break;
    }

    return ret;
}

void diversify () {
    int i;

    for (i = 0; i < config->number_of_query; i++) {
        diversifyQuery(i, config->diversification_algorithm, getNumberOfResults(i));
    }
}

void writeResults () {
    FILE *fp;
    int q_no, j;
    char results_path[FILEPATH_LENGTH] = "";
    size_t len1 = strlen(config->preresults_path);
    memcpy(results_path, config->preresults_path, len1);
    memcpy(results_path+len1, "_diversified", 12);

    if (!(fp = fopen(results_path, "w"))) {
        return;
    }

    for (q_no = 0; q_no < config->number_of_query; q_no++)
        for (j = 0; j < config->number_of_preresults; j++)
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

    int i, j;
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

    for (i = 0; i < config->number_of_query; i++) {
        for (j = 0; j < config->number_of_preresults; j++) {
            preresults[i][j].doc_id = 0;
            preresults[i][j].score = 0;
            results[i][j].doc_id = 0;
            results[i][j].score = 0;
        }
    }

    return 0;
}
