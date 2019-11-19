#include "word_embeddings.h"

void init_queries () {
    int i, j;
    for (i = 0; i < MAX_NOF_QUERIES; i++) {
        strcpy(queries[i].word, "");
        queries[i].norm = 0.0;
        for (j = 0; j < GLOVE_VECTOR_SIZE; j++) {
            queries[i].vector[j] = 0.0;
        }
    }
}

void init_dictionary () {
    int i, j;
    for (i = 0; i < GLOVE_DICT_SIZE; i++) {
        strcpy(dictionary[i].word, "");
        dictionary[i].norm = 0.0;
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
    double norm = 0.0;
    int d_index = 0, v_index = 0;

    if (!(fp = fopen(GLOVE_DATA_PATH, "r"))) {
        return -1;
    }

    while (!feof(fp)) {
        norm = 0.0;

        if (d_index >= GLOVE_DICT_SIZE) {
            printf("ERROR: THIS SHOULD NOT HAPPEN! %s\n", __FUNCTION__);
            break;
        }

        fscanf(fp, "%s ", word);
        strcpy(dictionary[d_index].word, word);
        for (v_index = 0; v_index+1 < GLOVE_VECTOR_SIZE; v_index++) {
            fscanf (fp, "%lf ", &(score));
            dictionary[d_index].vector[v_index] = score;
            norm += (score * score);
        }
        fscanf(fp, "%lf\n", &(score));
        dictionary[d_index].vector[v_index] = score;
        norm += (score * score);

        norm = sqrt(norm);
        dictionary[d_index].norm = norm;
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
    double norm = 0.0;

    if (!(fp = fopen(QUERIES_PATH_IN, "r"))) {
        printf("fopen %s failed.\n", QUERIES_PATH_IN);
        return -1;
    }

    do {
        norm = 0.0;

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
            norm += (queries[q_index].vector[v_index] * queries[q_index].vector[v_index]);
        }
        norm = sqrt(norm);
        queries[q_index].norm = norm;

        q_index++;
    } while (!feof(fp));

    real_nof_queries = q_index;
    if (real_nof_queries > MAX_NOF_QUERIES) {
        printf("ERROR: THIS SHOULD NOT HAPPEN! #queries:%d\n", real_nof_queries);
        return -1;
    }

    fclose(fp);
    return q_index;
}

int cmpsim (const void *a, const void *b) {
    Sim *simA = (Sim *)a;
    Sim *simB = (Sim *)b;
    double diff = simA->score - simB->score;

    if (diff > 0)
        return -1;
    else if (diff < 0)
        return 1;
    else
        return 0;
}

double cosine_similarity (We we1, We we2) {
    double score = 0.0;
    int v_index;
    double temp = 1;

    // If query word is not exists in dictionary, then we1 norm is 0
    if (we1.norm == 0)
        return 0;

    for(v_index = 0; v_index < GLOVE_VECTOR_SIZE; v_index++) {
        score += (we1.vector[v_index] * we2.vector[v_index]);
    }

    temp = (we1.norm * we2.norm);
    if (temp != 0) {
        score = score / temp;
    } else {
        printf("WARNING: POSSIBLE UNKNOWN WORD |%s|%s|\n",
            we1.word, we2.word);
        return 0;
    }

    return score;
}

int write_query (FILE *fp, int q_index, Sim *sims) {
    int s_index;
    int counter = 0;

    fprintf(fp, "%s ", queries[q_index].word);

    for (s_index = 0; (s_index < GLOVE_DICT_SIZE) && (counter < NOF_WORDS_TO_EXPAND); s_index++) {
        if (sims[s_index].index < 0) {
            printf("ERROR: THIS SHOULD NOT HAPPEN. sim-index:%d\n", sims[s_index].index);
            return -1;
        }

        if (sims[s_index].score == 1.0) {
            printf("WARNING: Possible 1-word query:%s. Skipping word:%s.\n",
                queries[q_index].word, dictionary[sims[s_index].index].word);
            continue;
        }

        fprintf(fp, "%s ", dictionary[sims[s_index].index].word);
        counter++;
    }
    fprintf(fp, "\n");

    return 0;
}

int expand_query (int q_index) {
    Similarity *query_word_similarities;
    int w_index = 0;

    size_t size = GLOVE_DICT_SIZE * sizeof(Sim);
    query_word_similarities = malloc(size);
    if (!query_word_similarities) {
        printf("query_word_similarities malloc failed\n");
        return -1;
    }
    memset(query_word_similarities, 0, size);

    for (w_index = 0; w_index < GLOVE_DICT_SIZE; w_index++) {
        query_word_similarities[w_index].index = w_index;
        query_word_similarities[w_index].score =
            cosine_similarity(queries[q_index], dictionary[w_index]);
    }

    qsort (query_word_similarities, GLOVE_DICT_SIZE, sizeof(Sim), cmpsim);

    if (write_query(qout_fp, q_index, query_word_similarities) != 0) {
        printf("writing q:%d failed\n", q_index);
        return -1;
    }

    // XXX diversified expansion

    free(query_word_similarities);
    return 0;
}
