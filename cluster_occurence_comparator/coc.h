#ifndef _COC_H_
#define _COC_H_

#include "../topic_based/c/Allocator.h"

#define COC_RESULT_FILENAME "coc_results.txt"

int getClusterId(int, int);
void getIncludedDocsFromInvertedIndex (int, int *);
unsigned int internal_comparison (long *, int);
bool readDocToClusterMap (int);
bool readDocToClusterMaps ();
void writeResults ();
void compare ();
int initCOC (Conf *);

#endif /* not defined _COC_H_ */
