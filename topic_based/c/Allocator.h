#ifndef _ALLOCATOR_H_
#define _ALLOCATOR_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "constants.h"

#define FILEPATH_LENGTH 255
#define MAX_TOKEN_LENGTH 8
/* <term-id, tf> each field is int (4 byte) */
#define TERM_ID_TF_PAIR_SIZE 8

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
    /* Term id starts from 0. */
    unsigned int term_id;
    /* Raw term text. */
    char token[MAX_TOKEN_LENGTH];
} Term, *Terms;

typedef struct TermVector {
    unsigned int term_id;
    unsigned int term_frequency;
} TermVector, *TermVectors;

typedef enum {
    SUCCESS,
    EMPTY_CONFIG_DATA,
    COULD_NOT_ALLOCATE_TERMS,
    COULD_NOT_ALLOCATE_DOCUMENTS,
    COULD_NOT_OPEN_WORDLIST,
    COULD_NOT_OPEN_DOCUMENT_INFO,
    NULL_DOCUMENT,
    COULD_NOT_ALLOCATE_TERM_VECTORS,
    COULD_NOT_OPEN_DOCUMENT_VECTORS_FILE
} State;

void actState ();
char *getDocumentVectorsFilepath (unsigned int);
TermVectors getTermVectors (Document*);
int loadDocuments ();
int loadTerms ();
int initAllocator (Conf*);

Conf *config;
Terms terms;
Documents documents;
State state;

#endif  /* not defined _ALLOCATOR_H_ */
