#ifndef _QP_BM25_H_
#define _QP_BM25_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <math.h>

#include "staticMaxHeap.h"

#define NOSTOPWORD 419
#define MAX_TUPLE_LENGTH 300000 // wrt. the tuple definition below...
#define WORD_NO 164000000  // SORUN: Bu  ne kadar buyuk olabilir? I hope it will work
#define TOKEN_NO  20000 // max no of tokens in a field: now it is token no at each doc line
#define TOKEN_SIZE 21 // max length of a token
#define DOC_SIZE 600000
#define DOC_NUM 50220539 /* exact number: 50220538 */ // get 1 more than exact value, so is word no

#define QSIZE 3000
#define BEST_DOCS 1000

#define NOF_Q 50 // TODO CHANGE IT 198 ? 200
#define MAX_SQ_PER_Q 8
#define MAX_SQ_LENGTH 1000

double BM25_K1_CONSTANT = 1.2;
double BM25_B_CONSTANT = 0.5;

typedef struct inv_ent {
    int doc_id;
    int weight;
} InvEntry;

typedef struct words {
    char a_word[TOKEN_SIZE]; // for handling queries
    int occurs_in_docs; // to retrieve inv entires and also weight queries
    off_t disk_address; // address to access in binary file
    InvEntry *postinglist;
} Word;

/****************************************/
/**************** Functions *************/
/****************************************/

int separator(char ch);
int FindStopIndex(int start,int end,char word[50]);

int cmpfunc(const void *, const void *);
int lex_order(char *, char *);
void read_next_value(char *);

/* this function would process TName field of a given tuple of a relation
 * processing means tokenzing the words in the TName and locating these in the tokens
 * field of the relatipn */
void process_tuple(char *);

/* initializes the doc vector for the *current* doc */
void initialize_doc_vec(int);
void initialize_accumulator();
void initialize_results();

/* inserts an accumulator into the set of top s accumulators,
 * if its score is higher than the minimum score in the heap */
void selection(double, int);

/* sorts a given heap of accumulators in decreasing order of their scores */
void sorting();

/* Method TOs4: A min-heap is used for selecting accumulators */
void TOs4ExtractionSelectionSorting(int);

void run_ranking_query(long int*, int);
void process_ranked_query(char *);

int load_subqueries(char *);

/****************************************/
/************ Global variables **********/
/****************************************/

struct staticMaxHeapStruct maxScoresHeap;

Word *WordList;
int word_no_in_list = 0;

char stopwords[NOSTOPWORD][50] ; // to keep stop words

long int *DVector; // [DOC_SIZE]; // to keep all term in a doc with dublications
int d_size = 0; // length of current doc

long int q_no = 0; // total no_of docs in all files

FILE * ifp, *eval_out;
FILE *entry_ifp, *out_trec;

char tName2[MAX_TUPLE_LENGTH];

Result *accumulator;
Result *results;

int *unique_terms;
int *total_tf_per_doc;
off_t unique_term_sum;
double avg_unique;
double avg_total_tf;
double collection_total_tf;
int REMAINING_DOC_NUM = 0;

char subqueries[NOF_Q][MAX_SQ_PER_Q][MAX_SQ_LENGTH];

#endif
