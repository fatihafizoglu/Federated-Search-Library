#ifndef ALLOCATOR_H
#define ALLOCATOR_H

#include <stdio.h>
#include <stdlib.h>

#define handle_error_en(en, msg) \
        do { errno = en; perror(msg); exit(EXIT_FAILURE); } while (0);

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
#ifdef DEBUG
    int doc_id;
#endif
    /* Raw document id between <DOCNO> and </DOCNO> */
    char *doc_no;
    /* Number of unique terms. */
    int uterm_count;
    /* Number of total terms. */
    int term_count;
    /* Beginning of a document's TermVectors inside the file. */
    int offset;
} Document, *Documents;

typedef struct Term {
#ifdef DEBUG
	/* Starts with 1. */
    int term_id;
    /* Raw term text. */
    char *token;
#endif
    /* Number of occurences of a Term in whole Collection. */
    int total_count;
    /* CFS Weight? */
    double cfc_weight;
} Term, *Terms;

typedef struct TermVector {
    int term_id;
    int term_frequency;
} DocumentVector, *DocumentVectors;

typedef enum {
	SUCCESS,
	FILE_NOT_FOUND,
	EMPTY_CONFIG_DATA,
	INIT_NULL_MALLOC
} Error;

static const char* error_messages[] = {
	"Successful operation.",
	"File not found.",
	"Empty configuration data.",
	"Malloc returns null at initialization."
};

int initAllocator (Conf*);
int loadTerms ();

Conf *config;
Terms terms;
Documents documents;

#endif
