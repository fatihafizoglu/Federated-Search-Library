#include "diversify.h"

int cmpfunc_score (const void * a, const void * b) {
    Result *resultA = (Result *)a;
    Result *resultB = (Result *)b;
    double diff = resultA->score - resultB->score;

    if (diff > 0)
        return -1;
    else if (diff < 0)
        return 1;
    else
        return 0;
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

    ret = ret / (getVectorLength(doc1->uterm_count, tf_idf1) * getVectorLength(doc2->uterm_count, tf_idf2));

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

double getSubqueryResult (int q_no, int subquery_index, int doc_id, int preresults_index) {
    double score = 0.0;
    int i = 0;

    /* Preresult index is -1 when caller is not traversing over preresults */
    if (preresults_index != -1 &&
        doc_id == subquery_results[q_no][subquery_index][preresults_index].doc_id) {

        score = subquery_results[q_no][subquery_index][preresults_index].score;
        return score;
    }
#ifdef DEBUG
    else if (preresults_index != -1) {
        printf("Unexpected Index MISMatch: %s\n", __FUNCTION__);
        fflush(stdout);
    }
#endif

    for (i = 0; i < config->number_of_preresults; i++) {
        if (doc_id == subquery_results[q_no][subquery_index][i].doc_id) {
            score = subquery_results[q_no][subquery_index][i].score;
        }
    }

#ifdef DEBUG
    if (score == 0.0) {
        printf("Score 0.0 for Q#%d SQ#%d DOC#%d INDEX#%d FNC:%s\n",
            q_no, subquery_index, doc_id, preresults_index, __FUNCTION__);
        fflush(stdout);
    }
#endif

    return score;
}

double getSubqueryNovelty (int q_no, int subquery_index, int result_size) {
    int i;
    double novelty = 1.0;

    for (i = 0; i < result_size; i++) {
        novelty = novelty *
            (1 - getSubqueryResult(q_no, subquery_index, results[q_no][i].doc_id, -1));
    }

    return novelty;
}

int getNumberOfSubqueries (int q_no) {
    int i = 0;

    for (i = 0; i < config->max_possible_number_of_subquery; i++) {
        /* Check only first result */
        if (subquery_results[q_no][i][0].doc_id == 0 &&
            subquery_results[q_no][i][0].score == 0.0) {
                return i;
        }
    }

    return config->max_possible_number_of_subquery;
}

int xquad_diverse (int q_no, int number_of_preresults, int number_of_results) {
    int i, j, index = -1;
    int result_size = 0;
    double sum_score = 0.0, max_score = 0.0, local_score = 0.0;
    int number_of_subqueries = 0;

    // calculate sum of scores for preresults, it is needed for normalization
    for (i = 0; i < number_of_preresults; i++) {
        sum_score = sum_score + preresults[q_no][i].score;
    }

    if (sum_score == 0.0) {
        state = UNSUCCESSFUL_DIVERSIFICATION;
        return 0;
    }

    number_of_subqueries = getNumberOfSubqueries(q_no);

#ifdef DEBUG
    printf("#Subqueries: %d\n", number_of_subqueries);
    printf("Sum of scores: %lf (for normalization)\n", sum_score);
    fflush(stdout);
#endif

    // collect results
    while (result_size < number_of_results) {
        max_score = 0.0;
        index = -1;

        // find document that maximize Fxquad(doc)
        for (i = 0; i < number_of_preresults; i++) {
            // calculate Fxquad(doc)

            // relevance(query-doc) part
            local_score = (1.0 - (config->lambda)) * (preresults[q_no][i].score / sum_score);

            // diverse part
            if (config->lambda != 0.0) {
                double diverse_score = 0.0;

                for (j = 0; j < number_of_subqueries; j++) {
                    double likelihood, relevance, novelty;
                    likelihood = 1.0 / number_of_subqueries;
                    relevance = getSubqueryResult(q_no, j, preresults[q_no][i].doc_id, i);
                    relevance = relevance / sum_score;
                    novelty = getSubqueryNovelty(q_no, j, result_size);

                    diverse_score = diverse_score + (likelihood * relevance * novelty);
                }

                local_score = local_score + ((config->lambda) * (diverse_score));

            }

            if (local_score > max_score) {
                max_score = local_score;
                index = i;
            }
        }

        if (index < 0) {
            break;
        }

        results[q_no][result_size].doc_id = preresults[q_no][index].doc_id;
        results[q_no][result_size].score = preresults[q_no][index].score;
        results[q_no][result_size].query_id = preresults[q_no][index].query_id;
        result_size++;
    }

    return result_size;
}

int maxsum_diverse (int q_no, int number_of_preresults, int number_of_results) {
    int i, j, index1, index2;
    int result_size = 0;
    double max_score = 0.0, max_distance = 0.0;
    double distances[number_of_preresults][number_of_preresults];

    memset(distances, 0, number_of_preresults * number_of_preresults * sizeof(double));
    max_score = preresults[q_no][0].score; /* Since rank 1 is the highest score.*/
#ifdef DEBUG
    printf("max_score %f\n", max_score);
    fflush(stdout);
#endif
    for (i = 0; i < number_of_preresults; i++) {
        for (j = 1; j < number_of_preresults; j++) {
            distances[i][j] = (1.0 - (config->lambda)) * ((preresults[q_no][i].score/max_score) + (preresults[q_no][j].score/max_score)) +
                               2.0 * (config->lambda) * (1.0 - cosineSimilarity(preresults[q_no][i].doc_id, preresults[q_no][j].doc_id));
        }
    }

    /* Avoid from odd. */
    if (number_of_results % 2 == 1)
        number_of_results++;

    while (result_size < number_of_results) {
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
#ifdef DEBUG
            printf("rank %d, index1 %d, index2 %d\n", result_size, index1, index2);
            fflush(stdout);
#endif
            results[q_no][result_size].doc_id = preresults[q_no][index1].doc_id;
            results[q_no][result_size].score = preresults[q_no][index1].score;
            results[q_no][result_size].query_id = preresults[q_no][index1].query_id;
            result_size++;
            if (index1 != index2) {
                results[q_no][result_size].doc_id = preresults[q_no][index2].doc_id;
                results[q_no][result_size].score = preresults[q_no][index2].score;
                results[q_no][result_size].query_id = preresults[q_no][index2].query_id;
            } else {
                results[q_no][result_size].doc_id = 0;
                results[q_no][result_size].score = 0;
                results[q_no][result_size].query_id = 0;
            }

            result_size++;
            for (i = 0; i < number_of_preresults; i++) {
                distances[i][index1] = 0;
                distances[index1][i] = 0;
                distances[i][index2] = 0;
                distances[index2][i] = 0;
            }
        } else {
            printf("Max-sum could not collect enough results.\n");
            break;
        }
    }
    return result_size;
}

int mmr_diverse (int q_no, int number_of_preresults, int number_of_results) {
    // TODO: implement MMR

    return 0;
}

int sy_diverse (int q_no, int number_of_preresults, int number_of_results) {
    int i = 0, j = 0;
    int result_size = 0;

    cleanPreresultsMarks();

    while (result_size < number_of_results && i < number_of_preresults) {
        if (preresults[q_no][i].doc_id == 0 || preresults[q_no][i].mark == true) {
            i++;
            continue;
        }

        j = i + 1;
        while (j < number_of_preresults) {
            if (preresults[q_no][j].doc_id == 0 || preresults[q_no][j].mark == true) {
                j++;
                continue;
            }

            if (cosineSimilarity(preresults[q_no][i].doc_id, preresults[q_no][j].doc_id) > (config->lambda)) {
                /* Mark result j. */
                preresults[q_no][j].mark = true;
            }
            j++;
        }

        if (preresults[q_no][i].mark == true) {
            results[q_no][result_size].doc_id = 0;
            results[q_no][result_size].score = 0;
            results[q_no][result_size].query_id = 0;
        } else {
            results[q_no][result_size].doc_id = preresults[q_no][i].doc_id;
            results[q_no][result_size].score = preresults[q_no][i].score;
            results[q_no][result_size].query_id = preresults[q_no][i].query_id;
        }

        result_size++;
        i++;
    }

    return result_size;
}

void diversifyQuery (int q_no, int algorithm, int number_of_preresults) {
    int result_size;

    int number_of_results = config->number_of_results;
    /* If preresults size smaller than expected results size, use preresults size. */
    if (number_of_preresults < number_of_results)
        number_of_results = number_of_preresults;

    if (algorithm == MAX_SUM) {
        result_size = maxsum_diverse(q_no, number_of_preresults, number_of_results);

    } else if (algorithm == MMR) {
        result_size = mmr_diverse(q_no, number_of_preresults, number_of_results);

    } else if (algorithm == SY) {
        result_size = sy_diverse(q_no, number_of_preresults, number_of_results);

    } else if (algorithm == XQUAD) {
        result_size = xquad_diverse(q_no, number_of_preresults, number_of_results);
    } else {
        /* Let's implement one more algorithm. */
        return;
    }

#ifdef DEBUG
    if (number_of_results == number_of_preresults) {
        printf("\nIMPORTANT WARNING\n");
        printf("Diversified results has the same amount of results with preresults (%d),\n", number_of_preresults);
        printf("and if you are going to sort this list, final results will be the same as preresults.\n");
        fflush(stdout);
    }
#endif

    qsort (results[q_no], result_size, sizeof(Result), cmpfunc_score);
}

int getExactNumberOfPreresults (int q_no) {
    int ret = 0;

    for (ret = 0; ret < config->number_of_preresults; ret++) {
        if (preresults[q_no][ret].doc_id == 0 || preresults[q_no][ret].score == 0)
            break;
    }
#ifdef DEBUG
    printf("%s -> %d\n", __FUNCTION__, ret);
    fflush(stdout);
#endif
    return ret;
}

void diversify () {
    int i;

#ifdef DEBUG
    printf("%u_%f|#Q:", config->diversification_algorithm, config->lambda);
#endif
    for (i = 0; i < config->real_number_of_query; i++) {
#ifdef DEBUG
        printf("%d.", i+1);
        fflush(stdout);
#endif
        diversifyQuery(i, config->diversification_algorithm, getExactNumberOfPreresults(i));
    }
#ifdef DEBUG
    printf("\n");
#endif
    state = SUCCESS;
}

void writeResults () {
    FILE *fp;
    int q_no, j;
    int exact_query_number = config->number_of_query;
    char results_path[FILEPATH_LENGTH] = "";
    char confstr[20] = "";
    char lambdastr[5] = "";
    size_t len1 = strlen(config->preresults_path);
    memcpy(results_path, config->preresults_path, len1);
    sprintf(lambdastr, "%.2f", config->lambda);

    if (config->diversification_algorithm == MAX_SUM) {
        memcpy(confstr, "_maxsum", 7);
        memcpy(confstr+7, lambdastr, 5);
    } else if (config->diversification_algorithm == SY) {
        memcpy(confstr, "_sy", 3);
        memcpy(confstr+3, lambdastr, 5);
    }  else if (config->diversification_algorithm == XQUAD) {
        memcpy(confstr, "_xquad", 6);
        memcpy(confstr+6, lambdastr, 5);
    }

    memcpy(results_path+len1, confstr, 20);

    if (!(fp = fopen(results_path, "w"))) {
        return;
    }

    if (config->real_number_of_query != 0) {
        exact_query_number = config->real_number_of_query;
    }

    for (q_no = 0; q_no < exact_query_number; q_no++)
        for (j = 0; j < config->number_of_results; j++)
            if (results[q_no][j].doc_id != 0 && results[q_no][j].score != 0) {
                //fprintf(fp, "%d\tQ0\t%d\t%d\t%lf\tfs\n", q_no + 1, results[q_no][j].doc_id, j + 1, results[q_no][j].score);
                fprintf(fp, "%d\tQ0\t%d\t%d\t%lf\tfs\n", results[q_no][j].query_id, results[q_no][j].doc_id, j + 1, results[q_no][j].score);
            }

    fclose(fp);
    state = SUCCESS;
}

void cleanSubqueryResults () {
    int i, j, k;

    for (i = 0; i < config->number_of_query; i++) {
        for (j = 0; j < config->max_possible_number_of_subquery; j++) {
            for (k = 0; k < config->number_of_preresults; k++) {
                subquery_results[i][j][k].doc_id = 0;
                subquery_results[i][j][k].score = 0.0;
            }
        }
    }
}

void cleanPreresultsMarks () {
    int i, j;

    for (i = 0; i < config->number_of_query; i++) {
        for (j = 0; j < config->number_of_preresults; j++) {
            preresults[i][j].mark = false;
        }
    }
}

void cleanPreresults () {
    int i, j;

    for (i = 0; i < config->number_of_query; i++) {
        for (j = 0; j < config->number_of_preresults; j++) {
            preresults[i][j].doc_id = 0;
            preresults[i][j].score = 0;
            preresults[i][j].query_id = 0;
            preresults[i][j].mark = false;
        }
    }
}

void cleanResults () {
    int i, j;

    for (i = 0; i < config->number_of_query; i++) {
        for (j = 0; j < config->number_of_results; j++) {
            results[i][j].doc_id = 0;
            results[i][j].score = 0;
            results[i][j].query_id = 0;
            results[i][j].mark = false;
        }
    }
}

void cleanAllResults () {
    cleanPreresults();
    cleanResults();
}

int loadSubqueryResults() {
    FILE *fp;
    unsigned int query_id, subquery_id, doc_id;
    unsigned int  prev_query_id = -1;
    unsigned int  prev_subquery_id = -1;
    unsigned int sresult_counter = 1;
    double score;

    if (!(fp = fopen(config->subqueryresults_path, "r"))) {
        return -1;
    }

    while (!feof(fp)) {
        fscanf (fp, "%u\t%u\t%u\t%lf\n", &(query_id), &(subquery_id),
            &(doc_id), &(score));

#ifdef DEBUG
        printf("query_id:%u, subquery_id:%u, doc_id:%u, score:%lf\n", query_id, subquery_id, doc_id, score);
        printf("Preresults[%u][%u].doc(%u) ?= Subquery[%u][%u][%u].doc(%u)\n",
            (query_id-1), (sresult_counter), (preresults[query_id-1][sresult_counter].doc_id),
            (query_id-1), (subquery_id-1), (sresult_counter), doc_id);
        printf("prev_query_id:%u, prev_subquery_id:%u\n", prev_query_id, prev_subquery_id);
        fflush(stdout);
#endif

        // detect and set sresult_counter
        // either query id or sq id is updated then reset rank counter
        if ( ((prev_subquery_id != subquery_id) && (prev_subquery_id != -1)) ||
             ( (prev_query_id != query_id) && (prev_query_id != -1)) ) {

            sresult_counter = 1;
#ifdef DEBUG
            printf("New list detected\n");
            fflush(stdout);
#endif
        }

        if ( (query_id > config->number_of_query) ||
             (subquery_id > config->max_possible_number_of_subquery) ||
             (sresult_counter > config->number_of_preresults) ||
             (doc_id != preresults[query_id-1][sresult_counter-1].doc_id) ) {

             printf("!!! THIS SHOULD NOT HAPPEN! 1=(%d)||(%d)||(%d)||(%d)\n",
                 (query_id > config->number_of_query),
                 (subquery_id > config->max_possible_number_of_subquery),
                 (sresult_counter > config->number_of_preresults),
                 (doc_id != preresults[query_id-1][sresult_counter-1].doc_id) );

             printf("query_id:%u, subquery_id:%u, doc_id:%u, score:%lf\n", query_id, subquery_id, doc_id, score);
             printf("Preresults[%u][%u].doc(%u) ?= Subquery[%u][%u][%u].doc(%u)\n",
                 (query_id-1), (sresult_counter-1), (preresults[query_id-1][sresult_counter-1].doc_id),
                 (query_id-1), (subquery_id-1), (sresult_counter-1), doc_id);
             fflush(stdout);
             continue;
        }

        subquery_results[query_id-1][subquery_id-1][sresult_counter-1].doc_id = doc_id;
        subquery_results[query_id-1][subquery_id-1][sresult_counter-1].score = score;


        prev_query_id = query_id;
        prev_subquery_id = subquery_id;
        sresult_counter++;
    }

    return 0;
}

int initSubqueryResults() {
    int i, j;
    long subquery_results_per_query_alloc_size = config->number_of_query * sizeof(SResult **);
    long subquery_results_per_subquery_alloc_size = config->max_possible_number_of_subquery * sizeof(SResult *);
    long subquery_results_per_preresults_alloc_size = config->number_of_preresults * sizeof(SResult);

    if (!(subquery_results = malloc(subquery_results_per_query_alloc_size))) {
        return -1;
    }

    for (i = 0; i < config->number_of_query; i++) {
        if (!(subquery_results[i] = malloc(subquery_results_per_subquery_alloc_size))) {
            return -1;
        }

        for (j = 0; j < config->max_possible_number_of_subquery; i++) {
            if (!(subquery_results[i][j] = malloc(subquery_results_per_preresults_alloc_size))) {
                return -1;
            }
        }
    }

    return 0;
}

int initloadSubqueryResults() {
    if (initSubqueryResults() != 0) {
        return -1;
    }

    cleanSubqueryResults();

    if (loadSubqueryResults() != 0) {
        return -1;
    }

    return 0;
}

void loadPreresults_forCDIV () {
    FILE *fp;
    char temp[100];
    unsigned int query_id, query_counter = 1, prev_query_id = -1;
    unsigned int document_id;
    unsigned int rank, previous_rank = 1, rank_counter = 1;
    unsigned int cluster_id = -1, prev_cluster_id = -1;
    double score;

    if (!(fp = fopen(config->preresults_path, "r"))) {
        return;
    }

    while (!feof(fp)) {
        fscanf (fp, "%u %s %u %u %lf %s %u\n", &(query_id), temp, &(document_id),
                                           &(rank), &(score), temp, &(cluster_id));

#ifdef DEBUG
        printf("query_id:%u, doc_id:%u, rank:%u, score:%lf\n", query_id, document_id, rank, score);
        printf("prev_rank:%u, rank:%u, query_counter:%u, rank_counter:%u\n",
                previous_rank, rank, query_counter, rank_counter);
        printf("cluster_id:%u, prev_cluster_id:%u\n", cluster_id, prev_cluster_id);
        fflush(stdout);
#endif

        // new smart query list detection
        if ( ( (prev_cluster_id != cluster_id) && (prev_cluster_id != -1) ) ||
             (  previous_rank > rank) ||
             ( (prev_query_id != query_id) && (prev_query_id != -1)) ) {
            query_counter++;
            rank_counter = 1;

#ifdef DEBUG
            printf("New query list detected\n");
            fflush(stdout);
#endif
        }

        if (rank_counter > config->number_of_preresults) {
            printf("!!! THIS SHOULD NOT HAPPEN! rank_counter > config->number_of_preresults\n");
            printf("!!! query_id:%u, doc_id:%u, rank:%u, score:%lf\n", query_id, document_id, rank_counter, score);
            printf("prev_rank:%u, rank:%u, query_counter:%u, rank_counter:%u\n",
                    previous_rank, rank, query_counter, rank_counter);
            fflush(stdout);
            continue;
        }

        if (query_counter > config->number_of_query) {
            printf("!!! THIS SHOULD NOT HAPPEN! query_counter > config->number_of_query\n");
            printf("!!! query_id:%u, doc_id:%u, rank:%u, score:%lf\n", query_id, document_id, rank_counter, score);
            printf("prev_rank:%u, rank:%u, query_counter:%u, rank_counter:%u\n",
                    previous_rank, rank, query_counter, rank_counter);
            fflush(stdout);
            continue;
        }

        preresults[query_counter-1][rank_counter-1].doc_id = document_id;
        preresults[query_counter-1][rank_counter-1].score = score;
        preresults[query_counter-1][rank_counter-1].query_id = query_id;

        previous_rank = rank;
        prev_query_id = query_id;
        prev_cluster_id = cluster_id;
        rank_counter++;
    }

    config->real_number_of_query = query_counter;

    printf("REAL #QUERY:%u\n", config->real_number_of_query);
    fflush(stdout);

    fclose(fp);
    state = SUCCESS;
}

void loadPreresults () {
#ifdef CDIV
    loadPreresults_forCDIV();
    return;
#else
    FILE *fp;
    char temp[100];
    unsigned int query_id, query_counter = 1, prev_query_id = -1;
    unsigned int document_id;
    unsigned int rank, previous_rank = 1, rank_counter = 1;
    double score;


    if (!(fp = fopen(config->preresults_path, "r"))) {
        return;
    }

    while (!feof(fp)) {
        fscanf (fp, "%u %s %u %u %lf %s\n", &(query_id), temp, &(document_id),
                                            &(rank), &(score), temp);

#ifdef DEBUG
        printf("query_id:%u, doc_id:%u, rank:%u, score:%lf\n", query_id, document_id, rank, score);
        printf("prev_rank:%u, query_counter:%u, rank_counter:%u, prev_query_id:%u\n",
                previous_rank, query_counter, rank_counter, prev_query_id);
        fflush(stdout);
#endif

        // smart query list detection
        if ( (  previous_rank > rank) ||
             ( (prev_query_id != query_id) && (prev_query_id != -1)) ) {
            query_counter++;
            rank_counter = 1;

#ifdef DEBUG
            printf("New query list detected\n");
            fflush(stdout);
#endif
        }

        if ((query_counter > config->number_of_query) ||
            (rank_counter > config->number_of_preresults) ||
            (query_counter != query_id) ||
            (rank_counter != rank)) {

            printf("!!! THIS SHOULD NOT HAPPEN! 1=(%d)||(%d)||(%d)||(%d)\n",
                (query_counter > config->number_of_query),
                (rank_counter > config->number_of_preresults),
                (query_counter != query_id),
                (rank_counter != rank));
            printf("!!! query_id:%u, doc_id:%u, rank:%u, score:%lf\n", query_id, document_id, rank, score);
            printf("prev_rank:%u, query_counter:%u, rank_counter:%u, prev_query_id:%u\n",
                    previous_rank, query_counter, rank_counter, prev_query_id);
            fflush(stdout);
            continue;
        }

        preresults[query_counter-1][rank_counter-1].doc_id = document_id;
        preresults[query_counter-1][rank_counter-1].score = score;
        preresults[query_counter-1][rank_counter-1].query_id = query_id;

        previous_rank = rank;
        prev_query_id = query_id;
        rank_counter++;
    }

    config->real_number_of_query = query_counter;

    printf("REAL #QUERY:%u\n", config->real_number_of_query);
    fflush(stdout);

    fclose(fp);
    state = SUCCESS;
#endif
}

int initDiversify (Conf *conf) {
    if (!conf) {
        state = EMPTY_CONFIG_DATA;
        return -1;
    }

    config = conf;

    int i;
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

    for (i = 0; i < config->number_of_query; i++) {
        if (!(preresults[i] = malloc(preresults_alloc_size))) {
            return -1;
        }
    }

    if (!(results = malloc(results_pointer_alloc_size))) {
        return -1;
    }

    for (i = 0; i < config->number_of_query; i++) {
        if (!(results[i] = malloc(results_alloc_size))) {
            return -1;
        }
    }

    cleanAllResults();

    state = SUCCESS;
    return 0;
}
