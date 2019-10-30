#ifndef _DIVERSIFY_H_
#define _DIVERSIFY_H_

#include "../../topic_based/c/Allocator.h"

enum Diversification_Algorithm {
    MAX_SUM, /* Max-sum dispersion. */
    MMR, /* Maximal marginal relevance. */
    SY,
    XQUAD
};

int cmpfunc_score (const void *, const void *);
double dotProduct (TermVectors, int, double *, TermVectors, int, double *);
double getVectorLength (int, double *);
double cosineSimilarity (int, int);
void getQueryScores(int, int, double *, double *);

double getSubqueryResult (int, int);
double getSubqueryNovelty (int, int, int);

int xquad_diverse (int, int, int);
int maxsum_diverse (int, int, int);
int mmr_diverse (int, int, int);
int sy_diverse (int, int, int);

void diversifyQuery (int, int, int);
int getExactNumberOfPreresults (int);
void diversify ();

void writeResults ();

void cleanSubqueryResults ();
void cleanPreresultsMarks();
void cleanPreresults();
void cleanResults();
void cleanAllResults ();

void initloadSubqueryResults();
void loadPreresults ();
int initDiversify (Conf *);

#endif /* not defined _DIVERSIFY_H_ */
