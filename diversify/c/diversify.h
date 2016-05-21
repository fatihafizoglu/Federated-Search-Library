#ifndef _DIVERSIFY_H_
#define _DIVERSIFY_H_

// TODO: comment functions and variables.

#include "../../topic_based/c/Allocator.h"

#define MAX_SUM_LAMBDA 0.5

enum Diversification_Algorithm {
    MAX_SUM, /* Max-sum dispersion. */
    MMR, /* Maximal marginal relevance. */
    SF
};

int cmpfunc_score (const void *, const void *);
double dotProduct (TermVectors, int, double *, TermVectors, int, double *);
double getVectorLength (int, double *);
double cosineSimilarity (int, int);
void getQueryScores(int, int, double *, double *);
void diversifyQuery (int, int, int);
int getExactNumberOfPreresults (int);
void diversify ();
void writeResults ();
void loadPreresults ();
int initDiversify (Conf *);

#endif /* not defined _DIVERSIFY_H_ */
