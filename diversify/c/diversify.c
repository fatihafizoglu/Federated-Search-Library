#include "diversify.h"

int cmpfunc_score (const void * a, const void * b) {
    Result *resultA = (Result *)a;
    Result *resultB = (Result *)b;

    return (resultA->score - resultB->score);
}

double dotProduct (TermVectors v1, int len1, double *tf_idf1, TermVectors v2, int len2, double *tf_idf2) {
    double ret = 0.0;
    int i = 0, j = 0;

    while (i < len1 && j < len2) {
        if (v1[i].term_id == v2[j].term_id) {
            ret += tf_idf1[i] * tf_idf2[j];
            i++;
            j++;
        } else if (v1[i].term_id < v2[j].term_id)
            i++;
        else if (v1[i].term_id > v2[j].term_id)
            j++;
    }

    return ret;
}

double getVectorLength (int length, double *tf_idf) {
    double ret = 0.0;
    int i;

    for (i = 0; i < length; i++)
        ret += tf_idf[i] * tf_idf[i];

    ret = sqrt(ret);
    return ret;
}

double cosineSimilarity (int doc1_id, int doc2_id) {
    double ret = 0.0;
    Document *doc1 = getDocument(doc1_id);
    Document *doc2 = getDocument(doc2_id);
    double tf_idf1[doc1->uterm_count];
    double tf_idf2[doc2->uterm_count];
    TermVectors doc1_term_vectors = getTermVectors(doc1, tf_idf1);
    TermVectors doc2_term_vectors = getTermVectors(doc2, tf_idf2);

    ret = dotProduct(doc1_term_vectors, doc1->uterm_count, tf_idf1,
                     doc2_term_vectors, doc2->uterm_count, tf_idf2);
    // printf("dotdone(%d,%d): %lf\n", doc1_id, doc2_id, ret);
    ret = ret / (getVectorLength(doc1->uterm_count, tf_idf1) * getVectorLength(doc2->uterm_count, tf_idf2));

    // printf("cosdone(%d,%d): %lf\n", doc1_id, doc2_id, ret);
    free(doc1_term_vectors);
    free(doc2_term_vectors);
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

void diversifyQuery (int q_no, int algorithm, int number_of_preresults) {
    if (algorithm == MAX_SUM) {
        /* If preresults size smaller than wanted results size, just return. */
        if (number_of_preresults < config->number_of_results)
            return;

        int i, j, index1, index2;
        int result_size = 0;
        double max_score = 0.0, max_distance = 0.0;
        // double sum_score = 0.0;
        double distances[number_of_preresults][number_of_preresults];

        memset(distances, 0, number_of_preresults * number_of_preresults * sizeof(double));
        // getQueryScores(q_no, number_of_preresults, &max_score, &sum_score);
        max_score = preresults[q_no][0].score; /* Since rank 1 is the highest score.*/

        for (i = 0; i < number_of_preresults; i++) {
            for (j = 1; j < number_of_preresults; j++) {
                distances[i][j] = (1.0 - MAX_SUM_LAMBDA) * ((preresults[q_no][i].score/max_score) + (preresults[q_no][j].score/max_score)) + 2.0 * MAX_SUM_LAMBDA * (1.0 - cosineSimilarity(preresults[q_no][i].doc_id, preresults[q_no][j].doc_id));
                // printf("dist[%d][%d]: %lf\n", i, j, distances[i][j]);
            }
        }

        /* Avoid from odd. */
        if (config->number_of_results % 2 == 1)
            config->number_of_results++;

        while (result_size < config->number_of_results) {
            max_distance = 0.0;
            index1 = 0; index2 = 0;
            for (i = 0; i < number_of_preresults; i++) {
                for (j = 1; j < number_of_preresults; j++) {
                    if (distances[i][j] > max_distance) {
                        max_distance = distances[i][j];
                        index1 = i;
                        index2 = j;
                    }
                }
            }

            if (max_distance > 0) {
                results[q_no][result_size].doc_id = preresults[q_no][index1].doc_id;
                results[q_no][result_size].score = preresults[q_no][index1].score;
                result_size++;
                results[q_no][result_size].doc_id = preresults[q_no][index2].doc_id;
                results[q_no][result_size].score = preresults[q_no][index2].score;
                result_size++;

                for (i = 0; i < number_of_preresults; i++) {
                    distances[i][index1] = 0;
                    distances[index1][i] = 0;
                    distances[i][index2] = 0;
                    distances[index2][i] = 0;
                }
            } else {
                printf("Max-sum could not collect enough results.");
                break;
            }
        }

        for (int k = 0; k < config->number_of_results; k++)
            printf("%d\tQ0\t%d\t%d\t%lf\tfs\n", q_no + 1, results[q_no][k].doc_id, k + 1, results[q_no][k].score);

        // FIXME: qsort can not sort!!!
        qsort (results[q_no], result_size, sizeof(Result), cmpfunc_score);

    } else if (algorithm == MMR) {
        // TODO: implement MMR

    } else if (algorithm == SF) {
        // TODO: implement SF

    } else {
        /* Let's implement one more algorithm. */
    }
}

int getExactNumberOfPreresults (int q_no) {
    int ret = 0;

    for (ret = 0; ret < config->number_of_preresults; ret++) {
        if (preresults[q_no][ret].doc_id == 0 || preresults[q_no][ret].score == 0)
            break;
    }

    return ret;
}

void diversify () {
    int i;

    for (i = 0; i < 1/*config->number_of_query*/; i++) {
        printf("Query: %d results diversifying.\n", i);
        diversifyQuery(i, config->diversification_algorithm, getExactNumberOfPreresults(i));
    }
    state = SUCCESS;
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
        for (j = 0; j < config->number_of_results; j++)
            if (results[q_no][j].doc_id != 0 && results[q_no][j].score != 0)
                fprintf(fp, "%d\tQ0\t%d\t%d\t%lf\tfs\n", q_no + 1, results[q_no][j].doc_id, j + 1, results[q_no][j].score);

    fclose(fp);
    state = SUCCESS;
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
    state = SUCCESS;
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

    long preresults_pointer_alloc_size = config->number_of_query * sizeof(Result *);
    long preresults_alloc_size = config->number_of_preresults * sizeof(Result);

    long results_pointer_alloc_size = config->number_of_query * sizeof(Result *);
    long results_alloc_size = config->number_of_results * sizeof(Result);

    if (!(terms = malloc(term_alloc_size))) {
        state = COULD_NOT_ALLOCATE_TERMS;
        return -1;
    }

    if (!(documents = malloc(documents_alloc_size))) {
        state = COULD_NOT_ALLOCATE_DOCUMENTS;
        return -1;
    }

    if (!(preresults = malloc(preresults_pointer_alloc_size))) {
        return -1;
    }

    for (int i = 0; i < config->number_of_query; i++) {
        if (!(preresults[i] = malloc(preresults_alloc_size))) {
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
        }

        for (j = 0; j < config->number_of_results; j++) {
            results[i][j].doc_id = 0;
            results[i][j].score = 0;
        }
    }

    state = SUCCESS;
    return 0;
}
