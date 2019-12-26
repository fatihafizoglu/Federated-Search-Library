#ifndef _WORD_EMBEDDINGS_H_
#define _WORD_EMBEDDINGS_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include <sys/time.h>
#include <errno.h>

/*******************/
/***** CONFIGS *****/
/*******************/
#define GLOVE_DATA_PATH "glove.6B.100d.txt"
#define QUERIES_PATH_IN "querylist_we_to_expand.txt"
#define QUERIES_PATH_OUT "querylist_we_expanded.txt"
#define QUERIES_DIVED_PATH_OUT "querylist_we_divexpanded.txt"


/*********************/
/***** CONSTANTS *****/
/*********************/
#define GLOVE_VECTOR_SIZE 100
#define GLOVE_DICT_SIZE 400000
#define MAX_WORD_SIZE 1000

/* Each query are expanded this nof words */
#define NOF_WORDS_TO_EXPAND 5
#define MAX_NOF_QUERIES 1020 // 1012 = 814 subquery + 198 query
#define MAX_NOF_WORD_FOREACH_QUERY 50

/*  */
#define DIVERSIFY_CAND_SET_LENGTH 50
#define LAMBDA 0.5

/************************/
/***** DEFINITIONS ******/
/************************/
typedef struct word_embedding {
    char word[MAX_WORD_SIZE];
    double vector[GLOVE_VECTOR_SIZE];
    double norm;
} We;

typedef struct similarity_score_accumulator {
    double score;
    int index;
} Similarity, Sim;

typedef struct word {
    char word[MAX_WORD_SIZE];
} word;

/*********************/
/***** FUNCTIONS *****/
/*********************/
int load_dictionary();
int load_queries();
int load_comb();
int expand_query(int);

/********************/
/***** GLOBALS ******/
/********************/
We dictionary[GLOVE_DICT_SIZE];
We queries[MAX_NOF_QUERIES];

FILE *qout_fp, *qdout_fp;
int real_nof_queries;

Similarity query_word_similarities[GLOVE_DICT_SIZE];

#endif /* not defined _WORD_EMBEDDINGS_H_ */
