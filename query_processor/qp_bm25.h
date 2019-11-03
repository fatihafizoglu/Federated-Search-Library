#ifndef _FSMEM18_H_
#define _FSMEM18_H_

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
#define BUFFERSIZE DOC_NUM

#define QSIZE 3000
#define BEST_DOCS 1000

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
    double CFCweight; // CFC weight of the word
    InvEntry *postinglist;
    double term_tf_sum; // for dirichlet
    int real_occurs_in_docs; // FOR EXACT RANKING IN PRUNED CASES
} *WP, Word;

typedef struct doc_ve {
    long int index;
    char word[70];
    long int rank_in_doc;
    double term_weight; //INDEXER5
    long int address; // address of where to start reading;
} DocVec, *DV;

int separator(char ch);
int FindStopIndex(int start,int end,char word[50]);

int index_order(DV dvec1, DV dvec2);
int lex_order(char *s1, char *s2);
void read_next_value(char *into);

/* this function would process TName field of a given tuple of a relation
 * processing means tokenzing the words in the TName and locating these in the tokens
 * field of the relatipn */
void process_tuple(char *line, long int tuple_no);

void extract_content(char *line, char *content);

/* initializes the doc vector for the *current* doc */
void initialize_doc_vec(int d_size);

void initialize_accumulator();
void initialize_results();

/* inserts an accumulator into the set of top s accumulators,
 * if its score is higher than the minimum score in the heap */
void selection(double score, int docId);

/* sorts a given heap of accumulators in decreasing order of their scores */
void sorting();

/* Method TOs4: A min-heap is used for selecting accumulators */
void TOs4ExtractionSelectionSorting(int q_size);

void run_ranking_query(DocVec *q_vec, int q_size, int q_no, char* original_q_no);
void process_ranked_query(char * rel_name);

void main(int argc,char *argv[]);

#endif
