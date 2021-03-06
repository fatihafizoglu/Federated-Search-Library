#include "dictionary.h"

/* dictionary initialization code used in both DictCreate and grow */
Dict internalDictCreate(int size) {
    Dict d;
    int i;

    d = malloc(sizeof(*d));

    assert(d != 0);

    d->size = size;
    d->n = 0;
    d->table = malloc(sizeof(struct elt *) * d->size);

    assert(d->table != 0);

    for (i = 0; i < d->size; i++)
      d->table[i] = 0;

    return d;
}

Dict DictCreate(void) {
    return internalDictCreate(INITIAL_SIZE);
}

void DictDestroy(Dict d) {
    int i;
    struct elt *e;
    struct elt *next;

    for (i = 0; i < d->size; i++) {
        for(e = d->table[i]; e != 0; e = next) {
            next = e->next;
            free(e);
        }
    }

    free(d->table);
    free(d);
}

/*
 * Each input bit affects each output bit with about 50% probability.
 * There are no collisions (each input results in a different output).
 * The algorithm is fast except if the CPU doesn't have a built-in
 * integer multiplication unit. Assuming int is 32 bit.
 * http://stackoverflow.com/a/12996028
 */
static unsigned int hash_function_u(unsigned int x) {
    unsigned int h = x;
    h = ((h >> 16) ^ h) * 0x45d9f3b;
    h = ((h >> 16) ^ h) * 0x45d9f3b;
    h = ((h >> 16) ^ h);

    return h;
}

static void grow(Dict d) {
    Dict d2;            /* new dictionary we'll create */
    struct dict swap;   /* temporary structure for brain transplant */
    int i;
    struct elt *e;

    d2 = internalDictCreate(d->size * GROWTH_FACTOR);

    for (i = 0; i < d->size; i++) {
        for (e = d->table[i]; e != 0; e = e->next) {
            /* note: this recopies everything */
            /* a more efficient implementation would
             * patch out the strdups inside DictInsert
             * to avoid this problem */
            DictInsert(d2, e->key, e->value);
        }
    }

    /* the hideous part */
    /* We'll swap the guts of d and d2 */
    /* then call DictDestroy on d2 */
    swap = *d;
    *d = *d2;
    *d2 = swap;

    DictDestroy(d2);
}

void
DictInsert(Dict d, const unsigned int key, const unsigned int value) {
    struct elt *e;
    unsigned int h;

    assert(key >= 0);
    assert(value >= 0);

    e = malloc(sizeof(*e));

    assert(e);

    e->key = key;
    e->value = value;

    h = hash_function_u(key) % d->size;

    e->next = d->table[h];
    d->table[h] = e;

    d->n++;

    /* grow table if there is not enough room */
    if(d->n >= d->size * MAX_LOAD_FACTOR) {
        grow(d);
    }
}

void
DictUpdateOrInsert(Dict d, const unsigned int key, const unsigned int value) {
    struct elt *e;

    assert(key >= 0);
    assert(value >= 0);

    for (e = d->table[hash_function_u(key) % d->size]; e != 0; e = e->next) {
        if(e->key == key) {
            /* got it */
            e->value = value;
            return;
        }
    }
    /* Key does not exist, then insert it. */
    DictInsert(d, key, value);
}

void
DictIncreaseOrInsert(Dict d, const unsigned int key, const unsigned int value) {
    struct elt *e;

    assert(key >= 0);
    assert(value >= 0);

    for(e = d->table[hash_function_u(key) % d->size]; e != 0; e = e->next) {
        if(e->key == key) {
            /* got it */
            e->value = e->value + value;
            return;
        }
    }
    /* Key does not exist, then insert it. */
    DictInsert(d, key, value);
}

const unsigned int DictSearch(Dict d, const unsigned int key) {
    struct elt *e;

    for (e = d->table[hash_function_u(key) % d->size]; e != 0; e = e->next) {
        if(e->key == key) {
            return e->value;
        }
    }

    return 0;
}

void DictDelete(Dict d, const unsigned int key) {
    struct elt **prev;
    struct elt *e;

    for (prev = &(d->table[hash_function_u(key) % d->size]);
        *prev != 0;
        prev = &((*prev)->next)) {
        if((*prev)->key == key) {
            /* got it */
            e = *prev;
            *prev = e->next;

            free(e);
            return;
        }
    }
}
