#include "word_embeddings.h"

void init_similarities () {
    int i;

    for (i = 0; i < GLOVE_DICT_SIZE; i++) {
        query_word_similarities[i].score = 0.0;
        query_word_similarities[i].index = -1;
    }
}

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
            } else {
                printf("WARNING: UNFOUND WORD: '%s' in q[%d]:%s\n",
                    c_ptr, q_index, queries[q_index].word);
            }

            c_ptr = strtok (NULL, " ");
        }

        if (nof_words_in_query == 0) {
            printf("WARNING: 0 word found in dict for query: %s\n", queries[q_index].word);
        } else {
            for (v_index = 0; v_index < GLOVE_VECTOR_SIZE; v_index++) {
                queries[q_index].vector[v_index] = vector[v_index] / nof_words_in_query;
                norm += (queries[q_index].vector[v_index] * queries[q_index].vector[v_index]);
            }
            norm = sqrt(norm);
        }

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

int write_query (FILE *fp, int q_index, Sim *sims, int length) {
    int s_index;
    int counter = 0;

    fprintf(fp, "%s ", queries[q_index].word);

    for (s_index = 0; (s_index < length) && (counter < NOF_WORDS_TO_EXPAND); s_index++) {
        if (sims[s_index].index < 0) {
            printf("ERROR: THIS SHOULD NOT HAPPEN. sim-index:%d\n", sims[s_index].index);
            return -1;
        }

        if (sims[s_index].score == 1.0) {
            printf("WARNING: Possible 1-word query:%s. Skipping word:%s.\n",
                queries[q_index].word, dictionary[sims[s_index].index].word);
            continue;
        }

        if (sims[s_index].score == 0.0 && sims[s_index].index == 0) {
            printf("ERROR: THIS CAN ONLY HAPPEN IF(%d,div).q#%d\n", length, q_index);
            continue;
        }

        fprintf(fp, "%s ", dictionary[sims[s_index].index].word);
        counter++;
    }
    fprintf(fp, "\n");

    return 0;
}

int is_word_exist (const char *search_word, const char *source) {
    char word[MAX_WORD_SIZE];
    char dict[MAX_WORD_SIZE];

    strcpy(word, search_word);
    strcpy(dict, source);

    char *c_ptr = strtok(dict, " ");

    while (c_ptr != NULL) {
        if (strcmp(word, c_ptr) == 0) {
            return 1;
        }

        c_ptr = strtok (NULL, " ");
    }

    return 0;
}

void gen_we (We *we, Sim *sims, int length) {
    char word[MAX_WORD_SIZE] = "";
    double vector[GLOVE_VECTOR_SIZE] = {};
    double norm = 0.0;
    int s_index, v_index;
    int nof_words = 0;

    for (s_index = 0; s_index < length; s_index++) {
        int index_to_add = sims[s_index].index;
        if (sims[s_index].score == 0.0 && index_to_add == -1) {
            continue;
        }

        nof_words++;
        for (v_index = 0; v_index < GLOVE_VECTOR_SIZE; v_index++) {
            vector[v_index] = vector[v_index] +
                (dictionary[index_to_add].vector[v_index]);
        }

        strcat(word, dictionary[index_to_add].word);
        strcat(word, " ");
    }

    if (nof_words == 0) {
        printf("ERROR: THIS SHOULD NOT HAPPEN!\n");
    }

    for (v_index = 0; v_index < GLOVE_VECTOR_SIZE; v_index++) {
        we->vector[v_index] = vector[v_index] / nof_words;
        norm += (we->vector[v_index] * we->vector[v_index]);
    }
    norm = sqrt(norm);
    we->norm = norm;
    strcpy(we->word, word);
}

int expand_query (int q_index) {
    int w_index = 0;

    init_similarities();

    for (w_index = 0; w_index < GLOVE_DICT_SIZE; w_index++) {
        query_word_similarities[w_index].index = w_index;

        if (is_word_exist(dictionary[w_index].word, queries[q_index].word)) {
            query_word_similarities[w_index].score = 0.0;
            // printf("WARNING: '%s' in q[%d]'%s', skipping...\n",
            //     dictionary[w_index].word, q_index, queries[q_index].word);
            continue;
        }

        query_word_similarities[w_index].score =
            cosine_similarity(queries[q_index], dictionary[w_index]);
    }

    qsort(query_word_similarities, GLOVE_DICT_SIZE, sizeof(Sim), cmpsim);
    if (write_query(qout_fp, q_index, query_word_similarities, GLOVE_DICT_SIZE) != 0) {
        printf("writing q:%d failed\n", q_index);
        return -1;
    }

    // DIVERSIFY
    Sim selected_words[NOF_WORDS_TO_EXPAND] = {};
    int tmp_i;
    for (tmp_i = 0; tmp_i < NOF_WORDS_TO_EXPAND; tmp_i++) {
        selected_words[tmp_i].index = -1;
        selected_words[tmp_i].score = 0.0;
    }
    selected_words[0].index = query_word_similarities[0].index;
    selected_words[0].score = query_word_similarities[0].score;

    int i, j;
    for (i = 1; i < NOF_WORDS_TO_EXPAND; i++) {
        int best_index = -1;
        double max_mmr_score = -1;
        for (j = 0; j < DIVERSIFY_CAND_SET_LENGTH; j++) {
            int cand_index = query_word_similarities[j].index;
            double cand_sim = query_word_similarities[j].score;

            int found = 0;
            int temp_index;
            for (temp_index = 0; temp_index < NOF_WORDS_TO_EXPAND; temp_index++) {
                if (selected_words[temp_index].index == cand_index) {
                    found = 1;
                    break;
                }
            }

            if (found) {
                // printf("WARNING: Word '%s' is already selected.\n",
                //     dictionary[cand_index].word);
                continue;
            }

            double mmr_score = 0.0;
            double query_similarity = cand_sim;
            double sim_amongst_selected = 0.0;
            We selected_words_we;

            gen_we(&selected_words_we, selected_words, NOF_WORDS_TO_EXPAND);
            sim_amongst_selected =
                cosine_similarity(selected_words_we, dictionary[cand_index]);

            mmr_score = ((1-LAMBDA) * query_similarity) +
                (LAMBDA * (1-sim_amongst_selected));

            if (mmr_score > max_mmr_score) {
                max_mmr_score = mmr_score;
                best_index = cand_index;
            }
        }

        if (best_index != -1) {
            selected_words[i].index = best_index;
            selected_words[i].score = max_mmr_score;
        }
    }

    qsort(selected_words, NOF_WORDS_TO_EXPAND, sizeof(Sim), cmpsim);
    if (write_query(qdout_fp, q_index, selected_words, NOF_WORDS_TO_EXPAND) != 0) {
        printf("writing qd:%d failed\n", q_index);
        return -1;
    }

    return 0;
}
