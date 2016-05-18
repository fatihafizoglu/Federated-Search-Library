#include "fsmem18.h"

int QUIT_STOP = 0; // stop condition with quit strategy is reached
int CONT_STOP = 0; // stop condition with continue strategy is reached
int nonzero_doc_acc = 0;

struct staticMaxHeapStruct maxScoresHeap;
timer process_time; // total time
double t_sum = 0;

Word *WordList;
InvEntry *buffer;

double score;
int found = 0;
int word_no_in_list=0;

char stopwords[NOSTOPWORD][50] ; // to keep stop words

DocVec *DVector; // [DOC_SIZE]; // to keep all term in a doc with dublications
int d_size=0; // length of current doc

long int doc_no = 0; // total no_of docs in all files

int err=0;

FILE * ifp, *eval_out;
FILE * out, *entry_ifp, *out_trec;

char tName2[MAX_TUPLE_LENGTH];

int processing_R = 1;
int processing_S = 0;

char final_tokens [TOKEN_NO][TOKEN_SIZE];

Result *accumulator;
Result *results;

int total_list_length = 0;
int total_node_access = 0;
int nonzero_acc_nodes = 0;

off_t sum_list_length = 0;
off_t sum_node_access = 0;
off_t sum_nonzero_acc_nodes = 0;

double disk_read_time_per_query = 0;
double sum_disk_read_time_per_query = 0;

extern double total_disk_time;
extern double total_sequential_access;
extern double total_random_access;

int *unique_terms;
int *total_tf_per_doc;
off_t unique_term_sum;
double avg_unique;
double avg_total_tf;
double collection_total_tf;
int REMAINING_DOC_NUM = 0;
int total_no_of_terms_in_query = 0;

int separator(char ch) {
    if (!isalnum(ch))
        return(1);
    else
        return(0);
}

int FindStopIndex(int start,int end,char word[50]) {
    int length = end - start + 1;
    int index = start + (length/2);

    if (length<=0)
        return (-1);
    else if (strcmp(word,stopwords[index])<0)
        return(FindStopIndex(start,index-1,word));
    else if (strcmp(word,stopwords[index])>0)
        return(FindStopIndex(index+1,end,word));
    else
        return(index);
}

double v_length(Vector *Vect) {
    int i;
    double sum = 0;

    for (i=0; i<Vect->nonzero_elements; i++)
        sum += Vect->VecElement[i].weight * Vect->VecElement[i].weight;

    return (sqrt(sum));
}

void v_normalize(Vector *v) {
    int i;
    double length = v_length(v);

    for (i=0; i<v->nonzero_elements; i++)
         v->VecElement[i].weight = v->VecElement[i].weight/length;
}

double v_dotProduct(Vector *v1, Vector *v2) {
    int i,k;
    double sum = 0;

    // IMPORTANT: assuming each index appears only once in a vector
    for (i=0; i<v1->nonzero_elements; i++)
        for (k=0; k<v2->nonzero_elements; k++)
            if (v1->VecElement[i].index == v2->VecElement[k].index) {
                sum += v1->VecElement[i].weight * v2->VecElement[k].weight;
                //break; // by assumption, no index (word) is duplicate in a vector
            };

    return (sum);
}

double v_VecCos( Vector *v1, Vector *v2) {
    double t = v_dotProduct(v1, v2); // as i normalize, no need any more /(v_length(v1)*v_length(v2));

    fprintf(out, "dot %lf length %lf cos is %lf\n",v_dotProduct(v1, v2),v_length(v1)*v_length(v2), t);

    return t;
}

int index_order(DV dvec1, DV dvec2) {
    if (dvec1->index > dvec2->index)
        return 1;
    if (dvec1->index < dvec2->index)
        return -1;

    return 0;
}


int lex_order(char *s1, char *s2) {
    return strcmp(s1, s2);
}

void read_next_value(char *into) {
    int i = 0, j = 0;
    char tempo[MAX_TUPLE_LENGTH];

    /* skip separators */
    while (separator(tName2[i]) && tName2[i] != NULL ) i++;

    while (!separator(tName2[i]) && tName2[i] != NULL) {
        into[j++]=tName2[i];
        i++;
    }

    into[j] = NULL; /* into contains a value as a string */

    j = 0;
    while(tName2[i]!=NULL) {
        tempo[j]=tName2[i];
        i++;
        j++;
    }

    tempo[j] = NULL;
    strcpy(tName2, tempo);
}

void process_tuple(char *line, long int tuple_no) {
    char *tName, *rName;
    int trie_key;
    char tokens[TOKEN_NO][TOKEN_SIZE];
    char str[MAX_TUPLE_LENGTH], temp[MAX_TUPLE_LENGTH], tmp[MAX_TUPLE_LENGTH];
    int token_found,i,q, tokens_left, index;
    WP word, sword;
    off_t add1, add2;
    int word_size;

    strcpy(tName2, line); // DO NOT change tName2, read_next_val ona gore yazilmis!!!
    token_found = 0;

    read_next_value(str);

    while  (  *str != NULL) {
        strcpy(tokens[token_found],str);
        token_found++;
        read_next_value(str);
    }

    sword = (WP) malloc(sizeof(Word));

    // omit the tokens that are found in the stop list
    tokens_left = 0;
    for (i=0; i<token_found; i++) {
        word = NULL;

        for (q=0; q<strlen(tokens[i]); q++)
            tokens[i][q] = tolower(tokens[i][q]);

#if (STOP_MODE)
        if (FindStopIndex(0,NOSTOPWORD-1,tokens[i]) == -1) {
#elif (!STOP_MODE)
        if (1) {
#endif
            strcpy(tmp, tokens[i]);
            strcpy(final_tokens[tokens_left], tokens[i]);
            found=0;
            if (processing_R) {
                if (sword == NULL) {
                    printf("could not allocate space\n");
                    exit(1);
                }

                strcpy(sword->a_word,tokens[i]);
                word = (WP) bsearch(sword->a_word ,WordList, word_no_in_list+1, sizeof(WordList[0]), lex_order);

                if (word) {
                    found = 1;
                    add1 = (off_t) WordList;
                    add2 = (off_t) word;
                    word_size = sizeof(Word);
                    // Note that, the expression gives exactly the array index of WordList, starting from 0 of course
                    index = (add2-add1)/word_size;
                    //free(sword);
                } else {
                    printf("bsearch could not found\n");
                }

                if (found) { // will be surely found!!!
                    // now add this term to the current doc vector
                    strcpy(DVector[d_size].word, tokens[i]);
                    DVector[d_size].index = index;
                    d_size++;

                    if (d_size > DOC_SIZE) {
                        printf("Doc size exceeds %d!\n", DOC_SIZE);
                        exit(1);
                    }
                } else { // not found
                    printf("!!!!!!!In the second pass, could not found an index %s in wordlist\n", tokens[i]);
                    err++;
                }
            }

            tokens_left++;
        }

    } //end_for

    free(sword);
    if (DETAILED_LOG) {
        fprintf(out, "%ld ", tuple_no);
        for (i=0; i<tokens_left; i++) {
            fprintf (out,"%s ",final_tokens[i]); // debugging
            fflush(out);
        }
        fprintf(out, "\n");
    }
}

void extract_content(char *line, char *content) {
    char tmp[MAX_TUPLE_LENGTH];
    int i, j = 0;
    int opened = 0; // to denote if a tag is open or not

    for (i=0; i<strlen(line); i++) {
        if (line[i] == '<')
            opened = 1;
        else if (line[i] == '>')
            opened = 0;
        else if (!opened) {
            tmp[j] = line[i];
            j++;
        }
    }

    tmp [j] = NULL;
    strcpy(content, tmp);
}

void initialize_doc_vec(int d_size) {
    int i;

    for (i=0; i<=d_size; i++) {
        strcpy(DVector[i].word,"");
        DVector[i].index = -1;
        DVector[i].rank_in_doc = -1;
        DVector[i].address = -1;
    }
}

double dvec_length(int size) {
    int i;
    double sum = 0;

    for (i=0; i<size; i++)
        sum += DVector[i].term_weight * DVector[i].term_weight;

    return (sqrt(sum));
}

void dvec_normalize (int size) {
    int i;
    double length = dvec_length(size);

    for (i=0; i<size; i++)
        DVector[i].term_weight = DVector[i].term_weight/length;
}

void initialize_accumulator() {
    int i;

    for (i = 0; i < DOC_NUM; i++) {
        accumulator[i].doc_index = i;
        accumulator[i].sim_rank = 0;
        accumulator[i].last_updated_by = 0;
    }
}

void initialize_results() {
    int i;

    for (i = 0; i < BEST_DOCS; i++) {
        results[i].doc_index = 0;
        results[i].sim_rank = 0;
    }
}

int ordering (RP p, RP q) {
    if (p->sim_rank > q->sim_rank)
        return -1;
    if (p->sim_rank < q->sim_rank)
        return 1;

    return 0;
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

    // if (maxScoresHeap.itemCount < maxScoresHeap.maxSize) {
    //     maxScoresHeap.items[maxScoresHeap.itemCount].doc_index = docId;
    //     maxScoresHeap.items[maxScoresHeap.itemCount].sim_rank = -score;
    //     maxScoresHeap.itemCount++;
    //
    //     if (maxScoresHeap.itemCount == maxScoresHeap.maxSize)
    //         buildMaxHeap(&maxScoresHeap);
    // } else {
    //     queryMaxMaxHeap(&maxScoresHeap, &minDocId, &minScore);
    //     /* new accumulator has a score higher than the minimum, so insert it */
    //     if (score > -minScore) {
    //         maxScoresHeap.items[0].doc_index = docId;
    //         maxScoresHeap.items[0].sim_rank = -score;
    //         heapifyMaxHeap(&maxScoresHeap, 0);
    //     }
    // }
    //
    // accumulator[docId].sim_rank = 0;
}

void sorting() {
    double score;
    int docId;
    int maxHeapSize;
    int i;

    initialize_results();
    // if (maxScoresHeap.itemCount < maxScoresHeap.maxSize)
    //     buildMaxHeap(&maxScoresHeap);

    maxHeapSize = maxScoresHeap.itemCount;
    for (i = 0; i < maxHeapSize; i++) {
        extractMaxHeap(&maxScoresHeap, &docId, &score);
        results[maxHeapSize-i-1].doc_index = docId;
        results[maxHeapSize-i-1].sim_rank = -score;
    }
}

void TOs4ExtractionSelectionSorting(int q_size) {
    int i;

    createMaxHeap(&maxScoresHeap, TOP_N);

    for (i = 1; i < DOC_NUM; i++)
#if (AND_MODE)
        if (accumulator[i].sim_rank && accumulator[i].last_updated_by == q_size)
#else
        if (accumulator[i].sim_rank)
#endif
        {
            nonzero_acc_nodes++;

#if ((SIM_MEASURE == TFIDF) || (SIM_MEASURE == MF8))
            accumulator[i].sim_rank /= doc_lengths[i];
#elif (SIM_MEASURE == CARMEL_SMART)
            accumulator[i].sim_rank /= (double)(sqrt(0.8*avg_unique + 0.2 *unique_terms[i]));
#elif (SIM_MEASURE == DIRICHLET)
            accumulator[i].sim_rank += total_no_of_terms_in_query * (double) logf( DIRICHLET_CONSTANT /( (double) DIRICHLET_CONSTANT + total_tf_per_doc[i]));
#endif
            selection(accumulator[i].sim_rank, i);
        }

#if (AND_MODE)
        else accumulator[i].sim_rank = 0;
#endif

    // TODO: diversify TOP_N documents in accumulator

    sorting();
}

void run_ranking_query(DocVec *q_vec, int q_size, int q_no, char* original_q_no) {
    int i, j, k;
    off_t address;
    int s_pt, e_pt;
    int saka;
    int temp_ind;
    double temp_sim;
    double doc_weight = 0;
    int WRITE_BEST_N;
    int doc_id;

    disk_read_time_per_query  = 0;
    total_list_length = 0;
    total_node_access = 0;
    nonzero_acc_nodes = 0;

// #if (AND_MODE)
    initialize_accumulator();
// #endif

    u_cleartimer(&process_time);
    u_starttimer(&process_time);

    if (DETAILED_LOG)
        fprintf(out, "Processing Q %d\n", q_no);

    QUIT_STOP = 0; // stop condition with quit strategy is reached
    CONT_STOP = 0; // stop condition with continue strategy is reached
    nonzero_doc_acc = 0;

    for (i = 0; i < q_size; i++) {
        u_stoptimer(&process_time);
        t_sum += process_time.time;
        WordList[q_vec[i].index].postinglist = (InvEntry *) malloc(sizeof(InvEntry)*WordList[q_vec[i].index].occurs_in_docs);
        address = WordList[q_vec[i].index].disk_address;
        fseeko(entry_ifp, address, 0);
        fread(WordList[q_vec[i].index].postinglist, sizeof(InvEntry), WordList[q_vec[i].index].occurs_in_docs, entry_ifp);

        u_cleartimer(&process_time);
        u_starttimer(&process_time);

#if (PRUNE_METHOD == NO_PRUNE)
        total_list_length += WordList[q_vec[i].index].occurs_in_docs;
        uncompressed_DISK_TIME(WordList[q_vec[i].index].occurs_in_docs);

#if (SIM_MEASURE == DIRICHLET)
        WordList[q_vec[i].index].term_tf_sum = 0;
        for (j=0; j<WordList[q_vec[i].index].occurs_in_docs; j++)
            WordList[q_vec[i].index].term_tf_sum += WordList[q_vec[i].index].postinglist[j].weight;
#endif

        for (j = 0; j < WordList[q_vec[i].index].occurs_in_docs; j++) {
            doc_weight = 0;
#if (SIM_MEASURE == TFIDF)
            doc_weight= ((double)(WordList[q_vec[i].index].postinglist[j].weight) * WordList[q_vec[i].index].CFCweight);
            accumulator[WordList[q_vec[i].index].postinglist[j]/*buffer[j]*/.doc_id].sim_rank += doc_weight * q_vec[i].term_weight;
#elif (SIM_MEASURE == MF8)
            doc_weight= 1 + (log((double)(WordList[q_vec[i].index].postinglist[j].weight)));
            accumulator[WordList[q_vec[i].index].postinglist[j]/*buffer[j]*/.doc_id].sim_rank += doc_weight * q_vec[i].term_weight;
#elif (SIM_MEASURE == CARMEL_SMART)
            doc_id = WordList[q_vec[i].index].postinglist[j].doc_id;
            //asagidakinin son kismi CFC ise formul daha efficient yazilabilir,ama biz CFC'yi log10 yapmis olabilirz
            // orjinal formul aldik -0.5 var, o yuzden CFC kullanilamadi...ya da o da FIX hesaplanmali effciency dsgerekirse
            doc_weight = (log2(1+WordList[q_vec[i].index].postinglist[j].weight) / (double) log2(1 + (total_tf_per_doc[doc_id]/(double) unique_terms[doc_id]))) * log2(DOC_NUM / (double)WordList[q_vec[i].index].occurs_in_docs);
            accumulator[doc_id].sim_rank += doc_weight * q_vec[i].term_weight;
            if (accumulator[doc_id].sim_rank == 0)
                printf ("ehh %d %d %lf\n", doc_id, WordList[q_vec[i].index].postinglist[j].weight, doc_weight);
#elif (SIM_MEASURE == BM25)
            doc_id = WordList[q_vec[i].index].postinglist[j].doc_id;
            doc_weight = WordList[q_vec[i].index].postinglist[j].weight;
            q_vec[i].term_weight = log( (REMAINING_DOC_NUM-WordList[q_vec[i].index].occurs_in_docs + 0.5) / (double)(WordList[q_vec[i].index].occurs_in_docs + 0.5));
#if (AND_MODE)
            if (accumulator[doc_id].last_updated_by == i) {
                accumulator[doc_id].sim_rank += q_vec[i].term_weight * (doc_weight * (BM25_K1_CONSTANT + 1)) / (doc_weight + BM25_K1_CONSTANT *((1-BM25_B_CONSTANT)+(BM25_B_CONSTANT*(total_tf_per_doc[doc_id]/avg_total_tf ))));
                accumulator[doc_id].last_updated_by++;
            } else {
                accumulator[doc_id].sim_rank=0;
                accumulator[doc_id].last_updated_by = -1;
            }
#else
            accumulator[doc_id].sim_rank += q_vec[i].term_weight * (doc_weight * (BM25_K1_CONSTANT + 1)) / (doc_weight + BM25_K1_CONSTANT *((1-BM25_B_CONSTANT)+(BM25_B_CONSTANT*(total_tf_per_doc[doc_id]/avg_total_tf ))));
#endif

#elif (SIM_MEASURE == DIRICHLET)
            doc_id = WordList[q_vec[i].index].postinglist[j].doc_id;
            doc_weight= (double)(collection_total_tf / (double) (WordList[q_vec[i].index].term_tf_sum * DIRICHLET_CONSTANT));
            accumulator[doc_id].sim_rank += (double) logf (1 + WordList[q_vec[i].index].postinglist[j].weight * doc_weight) ;
#endif
            total_node_access++;
        }

#elif (PRUNE_METHOD==QUIT)
        if (!QUIT_STOP) {
            total_list_length += WordList[q_vec[i].index].occurs_in_docs;
            uncompressed_DISK_TIME(WordList[q_vec[i].index].occurs_in_docs);

            for (j = 0; j<WordList[q_vec[i].index].occurs_in_docs; j++) {
                doc_weight=0;

#if (SIM_MEASURE == TFIDF)
                doc_weight= ((double)(WordList[q_vec[i].index].postinglist[j].weight) * WordList[q_vec[i].index].CFCweight);
#elif (SIM_MEASURE == MF8)
                doc_weight= 1 + (log((double)(WordList[q_vec[i].index].postinglist[j].weight)));
#endif
                if (accumulator[WordList[q_vec[i].index].postinglist[j]/*buffer[j]*/.doc_id].sim_rank==0)
                    nonzero_doc_acc++;

                accumulator[WordList[q_vec[i].index].postinglist[j]/*buffer[j]*/.doc_id].sim_rank += doc_weight * q_vec[i].term_weight;
                total_node_access++;
            }
        }
#elif (PRUNE_METHOD==CONT)
        total_list_length += WordList[q_vec[i].index].occurs_in_docs;
        uncompressed_DISK_TIME(WordList[q_vec[i].index].occurs_in_docs);

        for (j=0; j<WordList[q_vec[i].index].occurs_in_docs; j++) {
            if (!CONT_STOP) {
                doc_weight=0;

#if (SIM_MEASURE == TFIDF)
                doc_weight= ((double)(WordList[q_vec[i].index].postinglist[j].weight) * WordList[q_vec[i].index].CFCweight);
#elif (SIM_MEASURE == MF8)
                doc_weight= 1 + (log((double)(WordList[q_vec[i].index].postinglist[j].weight)));
#endif
                if (accumulator[WordList[q_vec[i].index].postinglist[j]/*buffer[j]*/.doc_id].sim_rank==0)
                    nonzero_doc_acc++;

                accumulator[WordList[q_vec[i].index].postinglist[j]/*buffer[j]*/.doc_id].sim_rank += doc_weight * q_vec[i].term_weight;
                total_node_access++;

            } else {
                if (accumulator[WordList[q_vec[i].index].postinglist[j]/*buffer[j]*/.doc_id].sim_rank != 0) {
                    doc_weight=0;

#if (SIM_MEASURE == TFIDF)
                    doc_weight= ((double)(WordList[q_vec[i].index].postinglist[j].weight) * WordList[q_vec[i].index].CFCweight);
#elif (SIM_MEASURE == MF8)
                    doc_weight= 1 + (log((double)(WordList[q_vec[i].index].postinglist[j].weight)));
#endif

                    accumulator[WordList[q_vec[i].index].postinglist[j]/*buffer[j]*/.doc_id].sim_rank += doc_weight * q_vec[i].term_weight;
                }
            }
        }
#endif

#if (PRUNE_METHOD == QUIT)
        if (nonzero_doc_acc>DOC_PRUNE_ACC)
            QUIT_STOP = 1;  // or we could directly say break the loop!
#elif (PRUNE_METHOD == CONT)
        if (nonzero_doc_acc>DOC_PRUNE_ACC)
            CONT_STOP = 1;
#endif
        free(WordList[q_vec[i].index].postinglist);
    }

    if (!USE_HEAP) {
        s_pt = 0;
        e_pt = DOC_NUM - 1;

        while (s_pt < e_pt) {
            while (accumulator[s_pt].sim_rank != 0)
                s_pt++;

            while (accumulator[e_pt].sim_rank == 0)
                e_pt--;

            if (s_pt < e_pt) {
                accumulator[s_pt].doc_index = accumulator[e_pt].doc_index;
                accumulator[s_pt].sim_rank = accumulator[e_pt].sim_rank;
                accumulator[e_pt].doc_index = 0;
                accumulator[e_pt].sim_rank = 0;
            }
        }
        qsort(accumulator, s_pt, sizeof(accumulator[0]), ordering);

    } else {
        TOs4ExtractionSelectionSorting(q_size);
    }

    sum_list_length += total_list_length;
    sum_node_access += total_node_access;
    sum_nonzero_acc_nodes += nonzero_acc_nodes;
    sum_disk_read_time_per_query += disk_read_time_per_query;

    u_stoptimer(&process_time);
    t_sum += process_time.time;

    for (j = 0; j < TOP_N; j++)
        if (results[j].sim_rank == 0)
            break;

    WRITE_BEST_N = j;
    for (j = 0; j < WRITE_BEST_N; j++) {
        fprintf(out_trec, "%d\tQ0\t%d\t%d\t%lf\tfs\n", q_no + 1, results[j].doc_index, j + 1, results[j].sim_rank);
        //accumulator[j].sim_rank = 0;
    }

    if (DETAILED_LOG) {
        fprintf(out, "While processing %d:", q_no);
        fprintf(out, "Computed disk access time: %11.3lf\n",disk_read_time_per_query);
    }
}

void process_ranked_query(char *rel_name) {
    char line[MAX_TUPLE_LENGTH];
    long int tmp;
    int  i,j,k,p;
    char temp_line[MAX_TUPLE_LENGTH];
    char content[MAX_TUPLE_LENGTH];
    int max_tf;
    DocVec q_vec[QSIZE];
    long int token_freq, next;
    int count;
    char header[30];
    char open_tag[30], close_tag[30];
    char doc_id[20];
    char original_doc_id[10];
    double avg_tf = 0; // for CARMEL

    if (!(ifp = fopen(rel_name, "rt"))) {
        printf("file not found!\n");
        return;
    }

    tmp = 0;
    processing_R = 1;
    fgets(line, MAX_TUPLE_LENGTH, ifp); // read blank line

    while (!feof(ifp)) {
        sprintf(original_doc_id,"%ld", doc_no);
        line[strlen(line)-1] = NULL;
        extract_content(line, content);
        process_tuple(content, doc_no);
        qsort(DVector, d_size, sizeof(DVector[0]), index_order);

        count = 0;
        token_freq=1;   // the freq. of a given token is set to 1 initially
        max_tf = -1;

#if (SIM_MEASURE == DIRICHLET)
        total_no_of_terms_in_query = 0;
        total_no_of_terms_in_query = d_size;
#endif
        for (i=0; i < d_size; i++) {
            next = i + 1;
            if (next<d_size && DVector[i].index == DVector[next].index)
                token_freq++;
            else {
                q_vec[count].index = DVector[i].index;
                q_vec[count].rank_in_doc = token_freq;
                count++;
                token_freq = 1;
            }

            if (token_freq > max_tf)
                max_tf = token_freq;
        }

        avg_tf = 0;
        for (i = 0; i < count; i++)
            avg_tf += q_vec[i].rank_in_doc;

        avg_tf /= count;

        // set query weights
        for (i = 0; i < count; i++) {
#if (SIM_MEASURE == TFIDF)
            // Indeed a term with rank 0 can not be in this vector...
            if (q_vec[i].rank_in_doc != 0)
                q_vec[i].term_weight = (0.5 + 0.5 * (q_vec[i].rank_in_doc/max_tf)) * WordList[q_vec[i].index].CFCweight;
            else
                q_vec[i].term_weight = 0;
#elif (SIM_MEASURE == MF8)
            if (q_vec[i].rank_in_doc != 0)
                q_vec[i].term_weight = q_vec[i].rank_in_doc * ((double)WordList[q_vec[i].index].CFCweight/*/  M_LOG2E*/);
            else
                q_vec[i].term_weight = 0;
#elif (SIM_MEASURE == CARMEL_SMART)
            if (q_vec[i].rank_in_doc != 0)
                q_vec[i].term_weight = (log2(1+q_vec[i].rank_in_doc)) / (log2(1+avg_tf));
            else
                q_vec[i].term_weight = 0;
#endif
            fflush(out);
        }

        for (p=0; p < RUN_NO; p++) {
            run_ranking_query(q_vec, count, doc_no, original_doc_id);
        }

        if (DETAILED_LOG)
            fprintf(out,"query:%d process time: %11.3lf\n",doc_no, process_time.time);

        initialize_doc_vec(d_size); // so I avoid fully initializing the doc vec each time
        d_size = 0;
        doc_no++;

        fgets(line, MAX_TUPLE_LENGTH, ifp);// read either EOF or blank
    }

    fclose (ifp);
    printf("Doc no: %ld %ld\n", doc_no, tmp);
}

void main(int argc,char *argv[]) {
    int i,j,k, t, q;
    char file_name[15], file_name2[15];
    char *f,*r, *m, pre, end;
    int FileNumber[13];
    int file_index;
    char line[MAX_TUPLE_LENGTH];
    char str[TOKEN_SIZE];
    int rank;
    int check_doc_num = 0;
    off_t address;

    WordList = (Word *) malloc(sizeof(Word)*WORD_NO);
    buffer = (InvEntry *) malloc(sizeof(InvEntry) * BUFFERSIZE);
    accumulator = (Result*) malloc(sizeof(Result) * DOC_NUM);
    DVector = (DocVec *) malloc(sizeof(DocVec) * DOC_SIZE);
    results = (Result*) malloc(sizeof(Result)* BEST_DOCS);

    if (!WordList)
        printf("could not allocate???\n");

    int doc_id;
    double doc_length;
    char query_file[1000];
    int total_no_of_postings =0 ;
    double avg_doc_length;
    char dummy[100];
    char look[10];

    total_random_access = 0;
    total_sequential_access = 0;
    total_disk_time = 0;

    strcpy(query_file, argv[4]);

    out = fopen("log.txt", "wt");
    out_trec = fopen("result.txt", "wt");
    ifp = fopen("stopword.lst","rt") ;

    for (i=0; i < NOSTOPWORD; i++)
        fscanf(ifp,"%s\n", stopwords[i]);

    fclose(ifp);

    for (i = 1; i < DOC_NUM; i++)
        doc_lengths[i] = 0;

#if (SIM_MEASURE == TFIDF)
    if (!(ifp = fopen("../createIIS/doc_lengths.txt","rt"))) {
        printf("doc_lengths file is not found!\n");
        exit(1);
    }
#elif (SIM_MEASURE == MF8)
    if (!(ifp = fopen("../dVector/doc_lengths_mf8.txt","rt"))) {
        printf("doc_lengths file is not found!\n");
        exit(1);
    }
#elif ((SIM_MEASURE == CARMEL_SMART) || (SIM_MEASURE == BM25) || (SIM_MEASURE == DIRICHLET) )
    if (!(ifp = fopen(argv[3],"rt"))) {
        printf("CARMEL SMART doc_lengths file is not found!\n");
        exit(1);
    }
#endif

#if ((SIM_MEASURE == TFIDF) || (SIM_MEASURE == MF8))
    i = 0;
    while (!feof(ifp)) {
        fscanf (ifp, "%d %lf\n", &doc_id, &doc_length);
        avg_doc_length += doc_length;
#if (SIM_MEASURE == TFIDF)
        doc_lengths[doc_id] = doc_length;
#elif (SIM_MEASURE == MF8)
        doc_lengths[doc_id] = sqrt (doc_length);
#endif
        i++;
    }

    printf("avg doc length (meaningful for MF8) is %lf\n", avg_doc_length/DOC_NUM);
    printf("no of docs in doc_lengths file is %d\n", i);
    printf("and last seen doc id is: %d\n", doc_id);

#elif ((SIM_MEASURE == CARMEL_SMART) || (SIM_MEASURE == BM25) || (SIM_MEASURE == DIRICHLET))
    REMAINING_DOC_NUM = 0;

    unique_terms = (int *) malloc(sizeof(int)*DOC_NUM);
    total_tf_per_doc = (int *) malloc(sizeof(int)*DOC_NUM);

    unique_term_sum = 0;
    collection_total_tf  = 0;

    for (i=1; i < DOC_NUM; i++) {
        fscanf(ifp, "%d %d %d\n", &doc_id, &(unique_terms[i]), &(total_tf_per_doc[i]));
        if (doc_id != i) {
            printf("hata var %d %d %d %d\n", i, doc_id,unique_terms[i], total_tf_per_doc[i] );
            exit(1);
        }

        if (unique_terms[i] > 0)
            REMAINING_DOC_NUM++;

        unique_term_sum += unique_terms[i];
        collection_total_tf += total_tf_per_doc[i];
        check_doc_num++;
    }

    if (check_doc_num != DOC_NUM-1) {
        printf("okunan length file line sayisi %d DOC_NUM degeri %d REM_DOC_NUM %d\n", check_doc_num, DOC_NUM, REMAINING_DOC_NUM);
        exit(1);
    }

    avg_unique = unique_term_sum /(double)REMAINING_DOC_NUM; //DOC_NUM;
    avg_total_tf = collection_total_tf /(double)REMAINING_DOC_NUM;  //DOC_NUM;

    printf("unique sum %lld avg unique per doc %lf\n", unique_term_sum, avg_unique);
#endif

    if (!(ifp = fopen(argv[1],"rt"))) {
        printf("wlist file not found! %s\n", argv[1]);
        exit(1);
    }

    word_no_in_list = 1;

    while (!feof(ifp)) {
        fscanf (ifp, "%s %d %lf\n", str, &(WordList[word_no_in_list].occurs_in_docs), &(WordList[word_no_in_list].CFCweight));
        strcpy(WordList[word_no_in_list].a_word, str);

        WordList[word_no_in_list].term_tf_sum = 0;

        for (q=0; q<strlen(WordList[word_no_in_list].a_word); q++)
            WordList[word_no_in_list].a_word[q] = tolower(WordList[word_no_in_list].a_word[q]);

        if (word_no_in_list>1)
            WordList[word_no_in_list].disk_address = WordList[word_no_in_list-1].disk_address+(sizeof(InvEntry)*WordList[word_no_in_list-1].occurs_in_docs);
        else
            WordList[word_no_in_list].disk_address = 0;

        total_no_of_postings += WordList[word_no_in_list].occurs_in_docs;
        word_no_in_list++;
    }

    fclose(ifp);

    // here word_no_in_list-1 as I start from 1
    printf("i initialized the word list with %d words.\n", word_no_in_list-1);
    printf ("there are %d postings in the IIS\n", total_no_of_postings);

    initialize_doc_vec(DOC_SIZE);
    if (!(entry_ifp = fopen(argv[2],"rb"))) {
        printf("Inverted Index file not found!\n");
        exit(1);
    }

    process_ranked_query(query_file);

    if (DETAILED_LOG)
        fprintf(out,"Total query process time: %11.3lf\n", t_sum/(float)(RUN_NO * QUERY_NO));

    fclose(out);

    eval_out = fopen("stats.txt", "wt");

    fprintf(eval_out,"no of results returned: top %d\n", TOP_N);
    fprintf(eval_out, "total and avg total list length (over all queries): %lld %lf\n",sum_list_length, sum_list_length/(double) (QUERY_NO*RUN_NO));
    fprintf(eval_out, "total and avg total node access (over all queries): %lld %lf\n",sum_node_access, sum_node_access/(double) (QUERY_NO*RUN_NO));
    fprintf(eval_out, "total and avg nonzero acc nodes(over all queries): %lld %lf\n",sum_nonzero_acc_nodes, sum_nonzero_acc_nodes/(double) (QUERY_NO*RUN_NO));
    fprintf(eval_out,"Total query process time: %11.3lf\n", t_sum/(float)(RUN_NO * QUERY_NO));
    fprintf(eval_out,"Computed (theoretically) disk read time per query (avg): %11.3lf\n", total_disk_time/(float)(RUN_NO*QUERY_NO));
    fprintf(eval_out,"Computed (theoretically) seq disk read time (avg): %11.3lf\n", total_sequential_access/(float)(RUN_NO*QUERY_NO));
    fprintf(eval_out,"Computed (theoretically) random disk read time (avg): %11.3lf\n", total_random_access/(float)(RUN_NO*QUERY_NO));
    fprintf(eval_out,"Total query process time: %11.3lf\n", t_sum/(float)(RUN_NO * QUERY_NO));
    fprintf(eval_out,"total and avg nonzero acc nodes(over all queries): %lld %lf\n",sum_nonzero_acc_nodes, sum_nonzero_acc_nodes/(double) (QUERY_NO*RUN_NO));

    fclose(eval_out);
    fclose(entry_ifp);

    free(WordList);
    free(buffer);

    // TODO: free allocated memory areas where it is possible.
}
