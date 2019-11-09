#ifndef _QP_BM25_H_
#define _QP_BM25_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <math.h>

#include "staticMaxHeap.h"

#define NOSTOPWORD 419
#define MAX_QUERY_LENGTH 1000 // as a character, total of all words
#define WORD_NO 164000000  // SORUN: Bu  ne kadar buyuk olabilir? I hope it will work
#define MAX_TOKEN_SIZE 21
#define MAX_WORD_PER_QUERY 50
#define DOC_NUM 50220539 /* exact number: 50220538 */ // get 1 more than exact value, so is word no

#define BEST_DOCS 1000

#define NOF_Q 198
#define MAX_SQ_PER_Q 8
#define MAX_SQ_LENGTH 1000

#define SUBQUERY_OUTPUT "subquery_results"

#define BM25_K1_CONSTANT 1.2
#define BM25_B_CONSTANT 0.5

typedef struct inv_ent {
    int doc_id;
    int weight;
} InvEntry;

typedef struct words {
    char a_word[MAX_TOKEN_SIZE]; // for handling queries
    int occurs_in_docs; // to retrieve inv entires and also weight queries
    off_t disk_address; // address to access in binary file
    InvEntry *postinglist;
} Word;

/****************************************/
/**************** Functions *************/
/****************************************/

/* this function would process TName field of a given tuple of a relation
 * processing means tokenzing the words in the TName and locating these in the tokens
 * field of the relatipn */
int process_tuple(char *);

void initialize_query_word_indexes();
void initialize_accumulator();
void initialize_results();

/* inserts an accumulator into the set of top s accumulators,
 * if its score is higher than the minimum score in the heap */
void selection(double, int);


/* Method TOs4: A min-heap is used for selecting accumulators */
void TOs4ExtractionSelectionSorting(int);

void run_ranking_subquery(long int *, int);
void run_ranking_query(long int*, int);
void process_ranked_query(char *);

int load_subqueries(char *);

/****************************************/
/************ Global variables **********/
/****************************************/

struct staticMaxHeapStruct maxScoresHeap;
char stopwords[NOSTOPWORD][50];

/* Wordlist, and number of words in Wordlist */
Word *WordList;
int word_no_in_list;

// Keep track of query's words indexes,
// term_weight will be computed on-the-fly
long int *QueryWordsIndexes;

// Global query no being processed
long int q_no;

FILE *ifp;
FILE *inverted_index_fp;
FILE *output_fp;
FILE *subquery_output_fp;

char rest_of_query[MAX_QUERY_LENGTH];

Result *accumulator;
Result *results;

int *total_tf_per_doc;
double avg_total_tf;
int REMAINING_DOC_NUM;


char subqueries[NOF_Q][MAX_SQ_PER_Q][MAX_SQ_LENGTH];



#endif
