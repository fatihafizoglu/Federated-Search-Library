#ifndef _DIVERSIFY_H_
#define _DIVERSIFY_H_

#include "../../topic_based/c/Allocator.h"
#include <unistd.h>

#define MAX_SUM_LAMBDA 0.5

enum { MAX_SUM, MMR, SY };

typedef struct QueryResult {
    int doc_id;
    double score;
} Result;


int main (int, char **);


Result **preresults, **results;

#endif /* not defined _DIVERSIFY_H_ */
