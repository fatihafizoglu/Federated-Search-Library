#ifndef _FSMEM18_H_
#define _FSMEM18_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <math.h>

#include "ulib.h"
#include "sglobal.h"
#include "staticMaxHeap.h"

#define STOP_MODE 1 //if 1, remove stopwords form the query otherwise keep them
#define USE_HEAP 1
#define AND_MODE 0

#define NO_PRUNE 0
#define STOP 1
#define QUIT 2
#define CONT 3

#define PRUNE_METHOD NO_PRUNE
#define DOC_PRUNE_ACC 20000

typedef struct vec {
    int index;
    double weight;
} *VP, VectorComp;

typedef struct vec2 {
    VectorComp VecElement[1];
    int nonzero_elements;
} Vector;

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

/* compute vector length */
double v_length(Vector *Vect);

void v_normalize(Vector *v);
double v_dotProduct(Vector *v1, Vector *v2);

/* compute the cosine of angle between two vectors */
double v_VecCos( Vector *v1, Vector *v2);

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

/* compute vector length */
double dvec_length(int size);

void dvec_normalize (int size);
void initialize_accumulator();
void initialize_results();

/* decreasing order */
int ordering (RP p, RP q);

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
