#include "qp_bm25.h"

int separator(char ch) {
    if (!isalnum(ch))
        return(1);
    else
        return(0);
}

int FindStopIndex(int start, int end, char word[50]) {
    int length = end - start + 1;
    int index = start + (length/2);

    if (length <= 0)
        return (-1);
    else if (strcmp(word, stopwords[index]) < 0)
        return(FindStopIndex(start, index-1, word));
    else if (strcmp(word, stopwords[index]) > 0)
        return(FindStopIndex(index+1, end, word));
    else
        return(index);
}

int cmpfunc (const void *a, const void *b) {
   return ( *(long int*)a - *(long int*)b );
}

int lex_order(const void *s1, const void *s2) {
    return strcmp((char *)s1, (char *)s2);
}

void read_next_value(char *into) {
    int i = 0, j = 0;
    char tempo[MAX_QUERY_LENGTH];

    /* skip separators */
    while (separator(rest_of_query[i]) && rest_of_query[i] != '\0' ) i++;

    while (!separator(rest_of_query[i]) && rest_of_query[i] != '\0') {
        into[j++] = rest_of_query[i];
        i++;
    }

    into[j] = '\0'; /* into contains a value as a string */

    j = 0;
    while(rest_of_query[i] != '\0') {
        tempo[j] = rest_of_query[i];
        i++;
        j++;
    }

    tempo[j] = '\0';
    strcpy(rest_of_query, tempo);
}

int process_tuple(char *line) {
    char tokens[MAX_WORD_PER_QUERY][MAX_TOKEN_SIZE];
    char str[MAX_QUERY_LENGTH];
    int token_found,i,q, tokens_left, index;
    Word *word, *sword;
    off_t add1, add2;
    int word_size;
    int found = 0;
    int nof_words_in_query = 0;

    strcpy(rest_of_query, line);
    token_found = 0;

    read_next_value(str);

    while  (*str != '\0') {
        strcpy(tokens[token_found], str);
        token_found++;
        read_next_value(str);
    }

    sword = (Word*) malloc(sizeof(Word));

    // omit the tokens that are found in the stop list
    tokens_left = 0;
    for (i = 0; i < token_found; i++) {
        word = NULL;

        for (q = 0; q < strlen(tokens[i]); q++)
            tokens[i][q] = tolower(tokens[i][q]);

        if (FindStopIndex(0, NOSTOPWORD-1, tokens[i]) == -1) {
            found = 0;

            if (sword == NULL) {
                printf("could not allocate space\n");
                exit(1);
            }

            strcpy(sword->a_word, tokens[i]);
            word = (Word*) bsearch(sword->a_word, WordList, (word_no_in_list + 1), sizeof(WordList[0]), lex_order);

            if (word) {
                found = 1;
                add1 = (off_t) WordList;
                add2 = (off_t) word;
                word_size = sizeof(Word);
                // Note that, the expression gives exactly the array index of WordList, starting from 0 of course
                index = (add2-add1)/word_size;
            } else {
                printf("bsearch could not found\n");
            }

            if (found) { // will be surely found!!!
                // now add this term to the current query vector
                QueryWordsIndexes[nof_words_in_query] = index;
                nof_words_in_query++;

                if (nof_words_in_query > MAX_WORD_PER_QUERY) {
                    printf("Doc size exceeds %d!\n", nof_words_in_query);
                    printf("THIS SHOULD NOT HAPPEN!\n");
                    exit(1);
                }
            } else { // not found
                printf("!!!!!!!In the second pass, could not found an index %s in wordlist\n", tokens[i]);
            }

            tokens_left++;
        }
    }

    free(sword);

    return nof_words_in_query;
}

void initialize_query_word_indexes() {
    int i;

    for (i = 0; i <= MAX_WORD_PER_QUERY; i++) {
        QueryWordsIndexes[i] = -1;
    }
}

void initialize_accumulator() {
    int i;

    for (i = 0; i < DOC_NUM; i++) {
        accumulator[i].doc_index = i;
        accumulator[i].sim_rank = 0;
    }
}

void initialize_results() {
    int i;

    for (i = 0; i < BEST_DOCS; i++) {
        results[i].doc_index = 0;
        results[i].sim_rank = 0;
    }
}

void selection(double score, int docId) {
    double minScore;
    int minDocId;

    if (insertMaxHeap(&maxScoresHeap, docId, (-1)*score) != 0) {
        /* means that heap is full */
        queryMaxMaxHeap(&maxScoresHeap, &minDocId, &minScore);
        if (score > -minScore) {
            extractMaxHeap(&maxScoresHeap, &minDocId, &minScore);
            insertMaxHeap(&maxScoresHeap, docId, (-1)*score);
        }
    }
}

void sorting() {
    double score;
    int docId;
    int maxHeapSize;
    int i;

    initialize_results();

    maxHeapSize = maxScoresHeap.itemCount;
    for (i = 0; i < maxHeapSize; i++) {
        extractMaxHeap(&maxScoresHeap, &docId, &score);
        results[maxHeapSize-i-1].doc_index = docId;
        results[maxHeapSize-i-1].sim_rank = -score;
    }
}

void TOs4ExtractionSelectionSorting(int q_size) {
    int i;

    createMaxHeap(&maxScoresHeap, BEST_DOCS);
    for (i = 1; i < DOC_NUM; i++) {
        if (accumulator[i].sim_rank) {
            selection(accumulator[i].sim_rank, i);
        }
    }

    sorting();
    freeMaxHeap(&maxScoresHeap);
}

void run_ranking_subquery(long int *sq_vec, int sq_size) {
    int i, j;
    off_t address;
    double doc_weight = 0;
    int doc_id;

    initialize_accumulator();

    for (i = 0; i < sq_size; i++) {
        WordList[sq_vec[i]].postinglist = (InvEntry *) malloc(sizeof(InvEntry) * WordList[sq_vec[i]].occurs_in_docs);
        address = WordList[sq_vec[i]].disk_address;
        fseeko(inverted_index_fp, address, 0);
        fread(WordList[sq_vec[i]].postinglist, sizeof(InvEntry), WordList[sq_vec[i]].occurs_in_docs, inverted_index_fp);

        for (j = 0; j < WordList[sq_vec[i]].occurs_in_docs; j++) {
            doc_weight = 0;
            doc_id = WordList[sq_vec[i]].postinglist[j].doc_id;
            doc_weight = WordList[sq_vec[i]].postinglist[j].weight;
            // don't need to store term weights
            double term_weight_of_q_vec_i = log( (REMAINING_DOC_NUM-WordList[sq_vec[i]].occurs_in_docs + 0.5) / (double)(WordList[sq_vec[i]].occurs_in_docs + 0.5));
            accumulator[doc_id].sim_rank += term_weight_of_q_vec_i * (doc_weight * (BM25_K1_CONSTANT + 1)) / (doc_weight + BM25_K1_CONSTANT *((1-BM25_B_CONSTANT)+(BM25_B_CONSTANT*(total_tf_per_doc[doc_id]/avg_total_tf ))));
        }

        free(WordList[sq_vec[i]].postinglist);
    }
}

void run_ranking_query(long int *q_vec, int q_size) {
    int i, j;
    off_t address;
    double doc_weight = 0;
    int WRITE_BEST_N;
    int doc_id;

    initialize_accumulator();

    for (i = 0; i < q_size; i++) {
        WordList[q_vec[i]].postinglist = (InvEntry *) malloc(sizeof(InvEntry) * WordList[q_vec[i]].occurs_in_docs);
        address = WordList[q_vec[i]].disk_address;
        fseeko(inverted_index_fp, address, 0);
        fread(WordList[q_vec[i]].postinglist, sizeof(InvEntry), WordList[q_vec[i]].occurs_in_docs, inverted_index_fp);

        for (j = 0; j < WordList[q_vec[i]].occurs_in_docs; j++) {
            doc_weight = 0;
            doc_id = WordList[q_vec[i]].postinglist[j].doc_id;
            doc_weight = WordList[q_vec[i]].postinglist[j].weight;
            // don't need to store term weights
            double term_weight_of_q_vec_i = log( (REMAINING_DOC_NUM-WordList[q_vec[i]].occurs_in_docs + 0.5) / (double)(WordList[q_vec[i]].occurs_in_docs + 0.5));
            accumulator[doc_id].sim_rank += term_weight_of_q_vec_i * (doc_weight * (BM25_K1_CONSTANT + 1)) / (doc_weight + BM25_K1_CONSTANT *((1-BM25_B_CONSTANT)+(BM25_B_CONSTANT*(total_tf_per_doc[doc_id]/avg_total_tf ))));
        }

        free(WordList[q_vec[i]].postinglist);
    }

    TOs4ExtractionSelectionSorting(q_size);

    for (j = 0; j < BEST_DOCS; j++)
        if (results[j].sim_rank == 0)
            break;

    WRITE_BEST_N = j;
    for (j = 0; j < WRITE_BEST_N; j++) {
        fprintf(output_fp, "%lu\tQ0\t%d\t%d\t%lf\tfs\n", q_no + 1, results[j].doc_index, j + 1, results[j].sim_rank);
    }


#ifdef XQUAD
    int subquery_index = 0;

    /* Gather subquery results */
    for (subquery_index = 0; subquery_index < MAX_SQ_PER_Q; subquery_index++) {
        if (strcmp(subqueries[q_no][subquery_index], "") == 0) {
            break;
        }

#ifdef DEBUG
        printf("Processing subquery: %s\n", subqueries[q_no][subquery_index]);
        fflush(stdout);
#endif
        initialize_query_word_indexes();
        int nof_words_in_subquery = process_tuple(subqueries[q_no][subquery_index]);
        qsort(QueryWordsIndexes, nof_words_in_subquery, sizeof(QueryWordsIndexes[0]), cmpfunc);

        int sq_count = 0, word_index = 0;
        long int sq_vec[MAX_WORD_PER_QUERY];

        // Eleminate duplicate words from subquery
        for (word_index = 0; word_index < nof_words_in_subquery; word_index++) {
            if ((word_index + 1) < nof_words_in_subquery &&
                QueryWordsIndexes[word_index] == QueryWordsIndexes[word_index + 1]) {

            }
            else {
                sq_vec[sq_count] = QueryWordsIndexes[word_index];
                sq_count++;
            }
        }

        run_ranking_subquery(sq_vec, sq_count);

        /* accumulator is ready with subquery results */
        /* Write collected subquery results as: */
        /* <query_id subquery_id doc_id score>\n */
        for (j = 0; j < WRITE_BEST_N; j++) {
            fprintf(subquery_output_fp, "%lu\t%u\t%u\t%lf\n",
                    q_no + 1, subquery_index, results[j].doc_index, accumulator[results[j].doc_index].sim_rank);
        }
    }

#endif
}

void process_ranked_query(char *rel_name) {
    char line[MAX_QUERY_LENGTH];
    int  i;
    long int q_vec[MAX_WORD_PER_QUERY];
    int count;

    if (!(ifp = fopen(rel_name, "rt"))) {
        printf("file not found!\n");
        return;
    }

    fgets(line, MAX_QUERY_LENGTH, ifp); // read blank line
    q_no = 0;

    while (!feof(ifp)) {
        line[strlen(line)-1] = '\0';
        int nof_words_in_query = process_tuple(line);
        qsort(QueryWordsIndexes, nof_words_in_query, sizeof(QueryWordsIndexes[0]), cmpfunc);

        count = 0;

        // Eleminate duplicate words from query
        for (i = 0; i < nof_words_in_query; i++) {
            if ((i + 1) < nof_words_in_query && QueryWordsIndexes[i] == QueryWordsIndexes[i + 1]) {

            }
            else {
                q_vec[count] = QueryWordsIndexes[i];
                count++;
            }
        }

        run_ranking_query(q_vec, count);

        initialize_query_word_indexes();
        q_no++;

        fgets(line, MAX_QUERY_LENGTH, ifp);
    }

    fclose (ifp);
    printf("Query no: %ld\n", q_no);
}

/*
 * Subquery files are in format of:
 * <query_no>\t<subquery>
 * query_no starts from 1
 * subqueries are already stop-word eleminated.
 */
int load_subqueries(char *sq_filename) {
    FILE *sq_file;
    unsigned int query_no, q_no_prev = -1;
    unsigned int sq_no = 0;
    char line[MAX_QUERY_LENGTH+2] = "";
    char subquery[MAX_QUERY_LENGTH] = "";

    if (!(sq_file = fopen(sq_filename, "r"))) {
        printf("Subqueries file not found.\n");
        return -1;
    }

    memset(subqueries, 0, MAX_NOF_QUERY *
            MAX_SQ_PER_Q * MAX_QUERY_LENGTH);

    do {
        if (fgets(line, MAX_QUERY_LENGTH+2, sq_file) == NULL) {
            break;
        }
        sscanf(line, "%u\t%[^\n]", &(query_no), subquery);
        if (query_no != q_no_prev) {
            sq_no = 0;
        }
        strcpy(subqueries[query_no-1][sq_no], subquery);
#ifdef DEBUG
        printf("subqueries[%u][%u]: %s\n", query_no-1, sq_no, subqueries[query_no-1][sq_no]);
        fflush(stdout);
#endif
        q_no_prev = query_no;
        sq_no++;
    } while (!feof(sq_file));

    fclose(sq_file);
    return 0;
}

#ifndef TESTER
int main(int argc,char *argv[]) {
    int i, q;
    char str[MAX_TOKEN_SIZE];
    int check_doc_num = 0;

    WordList = (Word*) malloc(sizeof(Word) * WORD_NO);
    accumulator = (Result*) malloc(sizeof(Result) * DOC_NUM);
    QueryWordsIndexes = (long int*) malloc(sizeof(long int) * MAX_WORD_PER_QUERY);
    results = (Result*) malloc(sizeof(Result)* BEST_DOCS);

    if (!WordList)
        printf("could not allocate???\n");

    int doc_id;
    char query_file[1000];

    strcpy(query_file, argv[4]);

    output_fp = fopen(argv[5], "wt");
    ifp = fopen("stopword.lst","rt");

    for (i = 0; i < NOSTOPWORD; i++)
        fscanf(ifp,"%s\n", stopwords[i]);

    fclose(ifp);

    if (!(ifp = fopen(argv[3],"rt"))) {
        printf("CARMEL SMART doc_lengths file is not found!\n");
        exit(1);
    }

    REMAINING_DOC_NUM = 0;

    int unique_terms = 0;
    total_tf_per_doc = (int *) malloc(sizeof(int) * DOC_NUM);

    double collection_total_tf = 0;
    for (i = 1; i < DOC_NUM; i++) {
        fscanf(ifp, "%d %d %d\n", &doc_id, &(unique_terms), &(total_tf_per_doc[i]));
        if (doc_id != i) {
            printf("hata var %d %d %d %d\n", i, doc_id, unique_terms, total_tf_per_doc[i]);
            exit(1);
        }

        if (unique_terms > 0)
            REMAINING_DOC_NUM++;

        collection_total_tf += total_tf_per_doc[i];
        check_doc_num++;
    }

    if (check_doc_num != DOC_NUM-1) {
        printf("okunan length file line sayisi %d DOC_NUM degeri %d REM_DOC_NUM %d\n", check_doc_num, DOC_NUM, REMAINING_DOC_NUM);
        exit(1);
    }

    avg_total_tf = collection_total_tf / (double)REMAINING_DOC_NUM;  //DOC_NUM;
    fclose(ifp);

    if (!(ifp = fopen(argv[1],"rt"))) {
        printf("wlist file not found! %s\n", argv[1]);
        exit(1);
    }

    word_no_in_list = 1;

    double CFCweight; // unused data

    while (!feof(ifp)) {
        fscanf (ifp, "%s %d %lf\n", str, &(WordList[word_no_in_list].occurs_in_docs),
            &(CFCweight));
        strcpy(WordList[word_no_in_list].a_word, str);

        for (q = 0; q < strlen(WordList[word_no_in_list].a_word); q++)
            WordList[word_no_in_list].a_word[q] = tolower(WordList[word_no_in_list].a_word[q]);

        if (word_no_in_list > 1)
            WordList[word_no_in_list].disk_address = WordList[word_no_in_list-1].disk_address +
                                                     (sizeof(InvEntry) * WordList[word_no_in_list-1].occurs_in_docs);
        else
            WordList[word_no_in_list].disk_address = 0;

        word_no_in_list++;
    }

    fclose(ifp);

    // here word_no_in_list-1 as I start from 1
    printf("i initialized the word list with %d words.\n", word_no_in_list-1);

    initialize_query_word_indexes();
    if (!(inverted_index_fp = fopen(argv[2],"rb"))) {
        printf("Inverted Index file not found!\n");
        exit(1);
    }

#ifdef XQUAD
    /* For xQuad, read subqueries. */
    if (load_subqueries(argv[6]) != 0) {
        printf("load_subqueries failed.\n");
        exit(1);
    }

    subquery_output_fp = fopen(SUBQUERY_OUTPUT, "wt");
#endif

    process_ranked_query(query_file);

    if (subquery_output_fp != NULL) {
        fclose(subquery_output_fp);
    }

    fclose(output_fp);
    fclose(inverted_index_fp);
    free(WordList);
    free(accumulator);
    free(QueryWordsIndexes);
    free(results);
    free(total_tf_per_doc);

    return 0;
}
#endif
