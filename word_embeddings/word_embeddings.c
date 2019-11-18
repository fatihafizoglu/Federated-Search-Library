#include "word_embeddings.h"

// double dotProduct (TermVectors v1, int len1, double *tf_idf1, TermVectors v2, int len2, double *tf_idf2) {
//     double ret = 0.0;
//     int i = 0, j = 0;
//
//     while (i < len1 && j < len2) {
//         if (v1[i].term_id == v2[j].term_id) {
//             ret += tf_idf1[i] * tf_idf2[j];
//             i++;
//             j++;
//         } else if (v1[i].term_id < v2[j].term_id)
//             i++;
//         else if (v1[i].term_id > v2[j].term_id)
//             j++;
//     }
//
//     return ret;
// }
//
// double getVectorLength (int length, double *tf_idf) {
//     double ret = 0.0;
//     int i;
//
//     for (i = 0; i < length; i++)
//         ret += tf_idf[i] * tf_idf[i];
//
//     ret = sqrt(ret);
//     return ret;
// }
//
// double cosineSimilarity (int doc1_id, int doc2_id) {
//     double ret = 0.0;
//     Document *doc1 = getDocument(doc1_id);
//     Document *doc2 = getDocument(doc2_id);
//     double tf_idf1[doc1->uterm_count];
//     double tf_idf2[doc2->uterm_count];
//     TermVectors doc1_term_vectors = getTermVectors(doc1, tf_idf1);
//     TermVectors doc2_term_vectors = getTermVectors(doc2, tf_idf2);
//
//     ret = dotProduct(doc1_term_vectors, doc1->uterm_count, tf_idf1,
//                      doc2_term_vectors, doc2->uterm_count, tf_idf2);
//
//     ret = ret / (getVectorLength(doc1->uterm_count, tf_idf1) * getVectorLength(doc2->uterm_count, tf_idf2));
//
//     free(doc1_term_vectors);
//     free(doc2_term_vectors);
//     return ret;
// }

void init_queries () {
    int i, j;
    for (i = 0; i < MAX_NOF_QUERIES; i++) {
        strcpy(queries[i].word, "");
        for (j = 0; j < GLOVE_VECTOR_SIZE; j++) {
            queries[i].vector[j] = 0.0;
        }
    }
}

void init_dictionary () {
    int i, j;
    for (i = 0; i < GLOVE_DICT_SIZE; i++) {
        strcpy(dictionary[i].word, "");
        for (j = 0; j < GLOVE_VECTOR_SIZE; j++) {
            dictionary[i].vector[j] = 0.0;
        }
    }
}

int load_dictionary () {
    init_dictionary();
    FILE *fp;
    char word[MAX_WORD_SIZE];
    double score;
    int d_index = 0, v_index = 0;

    if (!(fp = fopen(GLOVE_DATA_PATH, "r"))) {
        return -1;
    }

    while (!feof(fp)) {
        if (d_index >= GLOVE_DICT_SIZE) {
            printf("ERROR: THIS SHOULD NOT HAPPEN! %s\n", __FUNCTION__);
            break;
        }

        fscanf(fp, "%s ", word);
        strcpy(dictionary[d_index].word, word);
        for (v_index = 0; v_index+1 < GLOVE_VECTOR_SIZE; v_index++) {
            fscanf (fp, "%lf ", &(score));
            dictionary[d_index].vector[v_index] = score;
        }
        fscanf (fp, "%lf \n", &(score));
        dictionary[d_index].vector[v_index] = score;
        d_index++;
    }

    fclose(fp);
    return d_index;
}

int find_we (char *word) {
    int d_index;

    for (d_index = 0; d_index < GLOVE_DICT_SIZE; d_index++) {
        if (strcmp(word, dictionary[d_index].word) == 0) {
            break;
        }
    }

    return d_index;
}

int load_queries () {
    init_queries();
    FILE *fp;
    int q_index = 0, v_index = 0;
    int nof_words_in_query = 0;
    char line[MAX_WORD_SIZE+2] = "";
    char *c_ptr;

    if (!(fp = fopen(QUERIES_PATH_IN, "r"))) {
        return -1;
    }

    do {
        if (q_index >= MAX_NOF_QUERIES) {
            printf("ERROR: THIS SHOULD NOT HAPPEN! %s q_index\n",
                __FUNCTION__);
            break;
        }

        if (fgets(line, MAX_WORD_SIZE+2, fp) == NULL) {
            break;
        }
        line[strlen(line)-1] = '\0';
        strcpy(queries[q_index].word, line);
        nof_words_in_query = 0;
        c_ptr = strtok(line, " ");

        double vector[GLOVE_VECTOR_SIZE] = {};
        while (c_ptr != NULL) {
            int d_index = find_we(c_ptr);
            if (d_index < GLOVE_DICT_SIZE) {
                nof_words_in_query++;
                for (v_index = 0; v_index < GLOVE_VECTOR_SIZE; v_index++) {
                    vector[v_index] = vector[v_index] +
                        (dictionary[d_index].vector[v_index]);
                }
            }

            c_ptr = strtok (NULL, " ");
        }

        for (v_index = 0; v_index < GLOVE_VECTOR_SIZE; v_index++) {
            if (nof_words_in_query == 0) {
                printf("WARNING: 0 word found in dict for query: %s\n",
                    queries[q_index].word);
                break;
            }
            queries[q_index].vector[v_index] = vector[v_index] / nof_words_in_query;
        }

        q_index++;
    } while (!feof(fp));


    fclose(fp);
    return q_index;
}

double cosine_similarity (We we1, We we2) {
    double score = 0.0;

    // XXXboth we has same #nof vector compute score

    return score;
}


int expand_query (int q_index) {
    double *query_word_similarities;
    int w_index = 0;

    query_word_similarities = malloc(GLOVE_DICT_SIZE * sizeof(double));
    if (!query_word_similarities) {
        printf("query_word_similarities malloc failed\n");
        exit(1);
    }
    memset(query_word_similarities, 0, GLOVE_DICT_SIZE * sizeof(double));

    for (w_index = 0; w_index < GLOVE_DICT_SIZE; w_index++) {
        query_word_similarities[w_index] =
            cosine_similarity(queries[q_index], dictionary[w_index]);
    }

    // XXXfirst find the top k scores indexes from query_word_similarities
    // XXXthen, write the orig query to fp w/new k words from dictionary

    // XXX1st Change ABOVE algo w/diversified expansion

    free(query_word_similarities);
    return 0;
}
