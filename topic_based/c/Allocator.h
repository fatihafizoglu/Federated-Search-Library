#ifndef _ALLOCATOR_H_
#define _ALLOCATOR_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "constants.h"
#include "dictionary.h"

#define FILEPATH_LENGTH 255
#define NUMBER_OF_DOCUMENT_VECTORS_FILES 11

#define MAX_TOKEN_LENGTH 8
/* <term-id, tf> each field is int (4 byte) */
#define TERM_ID_TF_PAIR_SIZE 8
#define PERCENTAGE_OF_SAMPLES 0.01
#define NUMBER_OF_CLUSTERS 50

/* Configurable program elements. */
typedef struct AllocatorConfiguration {
    char *wordlist_path;
    char *document_info_path;
    char *document_vectors_folder_path;
    /* Total number of documents in the collection. */
    int number_of_documents;
    /* Total number of terms in the collection. */
    int number_of_terms;
} Conf;

typedef struct Cluster {
    unsigned long term_count;
    unsigned int *document_ids;
    unsigned int document_count;
    Dict dictionary;
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

/*
 * Calculates sample count and randomly generates sorted sample document ids.
 * Returns 0 if success, -1 in case of error.
 */
int sampleDocuments(double);

/*
 * Creates n number of clusters, also merged cluster.
 * Returns 0 if success, -1 in case of error and sets state.
 */
int initClusters (int);

/*
 * Returns term vectors of a given document.
 * Term vectors are lists of <term id, term frequency> pairs.
 */
TermVectors getTermVectors (Document*);

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
unsigned int *sample_doc_ids;
Cluster merged_cluster;

/*
 * TODO:
 * on program exit free these:
 * terms, documents, clusters, sample_doc_ids
 *
 * on function exits, free return values of these:
 * getTermVectors
 *
 * check all malloc statements, free all of them.
 *
 * define infile functions as static in allocator.
 *
 * const char* header'da static olmadan neden tanimlanamiyor. (multiple definition)
 */

#endif  /* not defined _ALLOCATOR_H_ */
