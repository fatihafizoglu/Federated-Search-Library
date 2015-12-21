#ifndef _DICTIONARY_H_
#define _DICTIONARY_H_

#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <string.h>

#define INITIAL_SIZE (1024)
#define GROWTH_FACTOR (2)
#define MAX_LOAD_FACTOR (1)

struct elt {
    struct elt *next;
    unsigned int key;
    unsigned int value;
};

typedef struct dict {
    int size;           /* size of the pointer table */
    int n;              /* number of elements stored */
    struct elt **table;
} *Dict;

/*
 * Create a new empty dictionary.
 */
Dict DictCreate(void);

/*
 * Destroy a dictionary.
 */
void DictDestroy(Dict);

/*
 * Insert a new key-value pair into an existing dictionary.
 */
void DictInsert(Dict d, const unsigned int key, const unsigned int value);

/*
 * Update given key-value pairs if key exists, o/w insert it.
 */
void DictUpdateOrInsert(Dict d, const unsigned int key, const unsigned int value);

/*
 * Increase given key's value if key exists, o/w insert it.
 */
void DictIncreaseOrInsert(Dict d, const unsigned int key, const unsigned int value);

/*
 * Return the most recently inserted value associated with a key,
 * or 0 if no matching key is present.
 */
const unsigned int DictSearch(Dict, const unsigned int key);

/*
 * Delete the most recently inserted record with the given key
 * if there is no such record, has no effect.
 */
void DictDelete(Dict, const unsigned int key);

#endif  /* not defined _ALLOCATOR_H_ */
