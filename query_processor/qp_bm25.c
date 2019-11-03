#include "qp_bm25.h"

struct staticMaxHeapStruct maxScoresHeap;

Word *WordList;
InvEntry *buffer;

int found = 0;
int word_no_in_list=0;

char stopwords[NOSTOPWORD][50] ; // to keep stop words

DocVec *DVector; // [DOC_SIZE]; // to keep all term in a doc with dublications
int d_size=0; // length of current doc

long int doc_no = 0; // total no_of docs in all files

FILE * ifp, *eval_out;
FILE *entry_ifp, *out_trec;

char tName2[MAX_TUPLE_LENGTH];

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

int *unique_terms;
int *total_tf_per_doc;
off_t unique_term_sum;
double avg_unique;
double avg_total_tf;
double collection_total_tf;
int REMAINING_DOC_NUM = 0;

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

        if (FindStopIndex(0,NOSTOPWORD-1,tokens[i]) == -1) {
            strcpy(tmp, tokens[i]);
            strcpy(final_tokens[tokens_left], tokens[i]);
            found=0;

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
            }

            tokens_left++;
        }

    } //end_for

    free(sword);
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
    for (i = 1; i < DOC_NUM; i++)
        if (accumulator[i].sim_rank)
        {
            nonzero_acc_nodes++;
            selection(accumulator[i].sim_rank, i);
        }

    sorting();
    freeMaxHeap(&maxScoresHeap);
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

    initialize_accumulator();


    for (i = 0; i < q_size; i++) {
        WordList[q_vec[i].index].postinglist = (InvEntry *) malloc(sizeof(InvEntry)*WordList[q_vec[i].index].occurs_in_docs);
        address = WordList[q_vec[i].index].disk_address;
        fseeko(entry_ifp, address, 0);
        fread(WordList[q_vec[i].index].postinglist, sizeof(InvEntry), WordList[q_vec[i].index].occurs_in_docs, entry_ifp);

        total_list_length += WordList[q_vec[i].index].occurs_in_docs;

        for (j = 0; j < WordList[q_vec[i].index].occurs_in_docs; j++) {
            doc_weight = 0;
            doc_id = WordList[q_vec[i].index].postinglist[j].doc_id;
            doc_weight = WordList[q_vec[i].index].postinglist[j].weight;
            q_vec[i].term_weight = log( (REMAINING_DOC_NUM-WordList[q_vec[i].index].occurs_in_docs + 0.5) / (double)(WordList[q_vec[i].index].occurs_in_docs + 0.5));

            accumulator[doc_id].sim_rank += q_vec[i].term_weight * (doc_weight * (BM25_K1_CONSTANT + 1)) / (doc_weight + BM25_K1_CONSTANT *((1-BM25_B_CONSTANT)+(BM25_B_CONSTANT*(total_tf_per_doc[doc_id]/avg_total_tf ))));
            total_node_access++;
        }

        free(WordList[q_vec[i].index].postinglist);
    }

    TOs4ExtractionSelectionSorting(q_size);

    sum_list_length += total_list_length;
    sum_node_access += total_node_access;
    sum_nonzero_acc_nodes += nonzero_acc_nodes;
    sum_disk_read_time_per_query += disk_read_time_per_query;

    for (j = 0; j < BEST_DOCS; j++)
        if (results[j].sim_rank == 0)
            break;

    WRITE_BEST_N = j;
    for (j = 0; j < WRITE_BEST_N; j++) {
        fprintf(out_trec, "%d\tQ0\t%d\t%d\t%lf\tfs\n", q_no + 1, results[j].doc_index, j + 1, results[j].sim_rank);
    }
}

void process_ranked_query(char *rel_name) {
    char line[MAX_TUPLE_LENGTH];
    long int tmp;
    int  i,j,k;
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

        run_ranking_query(q_vec, count, doc_no, original_doc_id);

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

    strcpy(query_file, argv[4]);

    out_trec = fopen(argv[5], "wt");
    ifp = fopen("stopword.lst","rt");

    for (i=0; i < NOSTOPWORD; i++)
        fscanf(ifp,"%s\n", stopwords[i]);

    fclose(ifp);

    if (!(ifp = fopen(argv[3],"rt"))) {
        printf("CARMEL SMART doc_lengths file is not found!\n");
        exit(1);
    }

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

    fclose(entry_ifp);

    free(WordList);
    free(buffer);
}
