#ifndef _DIVERSIFY_H_
#define _DIVERSIFY_H_

// TODO: comment functions and variables.

#include "../../topic_based/c/Allocator.h"

#define MAX_SUM_LAMBDA 0.5

enum Diversification_Algorithm {
    MAX_SUM, /* Max-sum dispersion. */
    MMR, /* Maximal marginal relevance. */
    SY
};

typedef struct QueryResult {
    int doc_id;
    double score;
} Result;

double cosineSimilarity (int, int);
void getQueryScores(int, int, double *, double *);
void diversifyQuery (int, int, int);
int getNumberOfResults (int);
void diversify ();
void writeResults ();
void loadPreresults ();
int initDiversify (Conf *);

Result **preresults, **results;

#endif /* not defined _DIVERSIFY_H_ */
