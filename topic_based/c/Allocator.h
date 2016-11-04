#ifndef _ALLOCATOR_H_
#define _ALLOCATOR_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include <sys/time.h>
#include "constants.h"
#include "dictionary.h"

#define FILEPATH_LENGTH 255
#define NUMBER_OF_DOCUMENT_VECTORS_FILES 11
#define CLUSTER_DOCUMENT_IDS_VECTOR_FILE_PREFIX "cluster"

#define MAX_TOKEN_LENGTH 8
/* <term-id, tf> each field is int (4 byte) */
#define TERM_ID_TF_PAIR_SIZE 8
/* <doc-id, occurence_count> each field is int (4 byte) */
#define POSTING_SIZE 8

#define PERCENTAGE_OF_SAMPLES 0.01
#define NUMBER_OF_CLUSTERS 100
#define LAMBDA 0.1

typedef enum { false, true } bool;

/* Configurable program elements. */
typedef struct AllocatorConfiguration {
    /* Files */
    char *inverted_index_path;
    char *wordlist_path;
    char *document_info_path;
    char *document_vectors_folder_path;

    /* Total number of documents in the collection. */
    unsigned int number_of_documents;
    /* Total number of terms in the collection. */
    unsigned int number_of_terms;


    /* ___DIVERSIFICATION___ */
    /* Files */
    char *preresults_path;
    char *results_path;
    /* Below ones required for result diversification process. */
    /* Set flag true if Diversify otherwise false. */
    bool DIVERSIFY;
    /* Total number of results per query in preresults. */
    unsigned int number_of_preresults;
    /* Total number of results per query in results. */
    unsigned int number_of_results;
    /* Total number of query in preresults. */
    unsigned int number_of_query;
    /* Selected diversification algorithm. */
    unsigned int diversification_algorithm;
    /* Diversification lambda value */
    double lambda;
    /* ___END___ */

    /* ___CLUSTER_OCCURENCE_COMPARATOR___ */
    /* Files */
    char *doc_to_cluster_map_path_for_c1;
    char *doc_to_cluster_map_path_for_c2;
    /* Number of clusters for both clusters {c1, c2}. */
    unsigned int number_of_clusters_for_c1;
    unsigned int number_of_clusters_for_c2;
    /* ___END___ */
} Conf;

typedef struct Cluster {
    unsigned long term_count;
    unsigned long new_term_count;
    Dict dictionary;
    Dict new_dictionary;
} Cluster, *Clusters;

typedef struct Document {
    /* Doc id starts from 1. */
    unsigned int doc_id;
    /* Number of unique terms. */
    unsigned int uterm_count;
    /* Number of total terms. */
    unsigned int term_count;
    /* Beginning of a document's term vectors inside the file. */
    unsigned int offset;
    /* Raw document id between <DOCNO> and </DOCNO> */
    char *doc_no;
} Document, *Documents;

typedef struct Term {
    /* CFS Weight? */
    double cfc_weight;
    /* Number of occurences of a term in whole collection. */
    unsigned int total_count;
    // TODO: termvector'ler dvec.bin'lerde hangi termid baslangiciyla listeli.
    /* Term id starts from 0. */
    unsigned int term_id;
    /* Raw term text. */
    char token[MAX_TOKEN_LENGTH];
} Term, *Terms;

typedef struct TermVector {
    unsigned int term_id;
    unsigned int term_frequency;
} TermVector, *TermVectors;

typedef struct QueryResult {
    int doc_id;
    double score;
} Result;

typedef struct Posting {
    int doc_id;
    int occurence_count;
} Posting;
/*
 * Frees all allocated memory blocks and closes opened files.
 */
void endProgram ();

/*
 * Returns true if sampled document ids array contains given document id.
 * o/w returns false.
 */
bool isDocumentSampled (unsigned int);

/*
 * For each unsampled documents, calculates similarity
 * with clusters and assign it to most similar one.
 */
void assignDocumentsToClusters ();

/*
 * Writes document id to cluster's document ids vector file.
 */
int writeDocumentIdToClusterFile (unsigned int, unsigned int);

/*
 * Returns document vectors file.
 */
FILE* getDocumentVectorsFile (unsigned int);

/*
 * Returns term vectors of a given document.
 * Term vectors are lists of <term id, term frequency> pairs.
 */
TermVectors getTermVectors (Document*, double*);

/*
 * Adds document to cluster's temporary dictionary.
 */
void addDocumentToCluster(Cluster*, Document*);

/*
 * Returns the pointer to the document that has given document id.
 */
Document *getDocument(unsigned int);

/*
 * Comparison function for quick sort.
 */
int cmpfunc (const void *, const void *);

/*
 * Returns values in the range [min, max], where
 * max >= min and 1+max-min < RAND_MAX
 */
unsigned int rand_interval(unsigned int, unsigned int);

/*
 * Puts sample_count random integer to sample_indeces in range (1, max).
 */
void randomSample (unsigned int *, unsigned int, unsigned int);

/*
 * After each kmeans run, generated dictionary is swapped with cluster's own dictionary.
 */
void swapDictionary ();

/*
 * For each sampled document, similarity is calculated for cluster's centroid.
 * Then document added to corresponding cluster and merged cluster.
 */
void kMeans();

/*
 * To run kmeans algorithm over clusters, first put 1 document to each cluster.
 */
void initializeKMeans ();

/*
 * Calculates sample count and randomly generates sorted sample document ids.
 * Returns 0 if success, -1 in case of error.
 */
int sampleDocuments();

/*
 * Creates n number of clusters, also merged cluster.
 * Returns 0 if success, -1 in case of error and sets state.
 */
int initClusters ();

/*
 * Reads documents info file and fills documents.
 * Returns number of documents loaded, calculate
 * offset wrt unique term counts.
 *
 * Note that: Document vectors seperated as: [#document id - #document id]
 * File 1 : [1          - 4 999 999]
 * File 2 : [5 000 000  - 9 999 999]
 * File 3 : [10 000 000 - 14 999 999]
 * ...
 * File 10 : [45 000 000 - 49 999 999]
 * File 11 : [50 000 000 - 50 220 538]
 */
int loadDocuments ();

/*
 * Reads merged wordlist file and fills terms.
 * Returns number of terms loaded.
 */
int loadTerms ();

/*
 * Closes all document vectors file.
 */
void closeDocumentVectorsFiles ();

/*
 * Closes all cluster document ids vector files.
 */
void closeClusterDocumentIdsFiles ();

/*
 * Returns document vectors file path as a string.
 */
char *getDocumentVectorsFilepath (unsigned int);

/*
 * Returns cluster's document ids vector file path as a string.
 */
char *getClusterDocumentIdsFilepath (unsigned int);

/*
 * Opens all cluster document ids vector files.
 */
int openClusterDocumentIdsFiles ();

/*
 * Opens all document vectors files once.
 */
int openDocumentVectorsFiles ();

/*
 * Should be called after any operation ends.
 * Checks program state and act according to this state.
 */
void actState ();

/*
 * Takes paths of a merged wordlist file, document info file and
 * folder containing document vectors files (i.e. dvec.bin).
 * Returns 0 if success, -1 otherwise and sets state.
 */
int initAllocator (Conf*);

Conf *config;
Terms terms;
Documents documents;
Clusters clusters;
State state;
FILE **document_vectors_files;
FILE **cluster_document_ids_files;
unsigned int *sample_doc_ids;
Cluster merged_cluster;
Result **preresults, **results;
unsigned int *terms_coc_counts;
FILE *inverted_index;
FILE *doc_to_cluster_map_for_c1_file;
FILE *doc_to_cluster_map_for_c2_file;
int *doc_to_cluster_map_for_c1;
int *doc_to_cluster_map_for_c2;

#endif  /* not defined _ALLOCATOR_H_ */
