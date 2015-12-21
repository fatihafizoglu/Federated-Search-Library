typedef struct dict *Dict;

/* create a new empty dictionary */
Dict DictCreate(void);

/* destroy a dictionary */
void DictDestroy(Dict);

/* insert a new key-value pair into an existing dictionary */
void DictInsert(Dict d, const unsigned int key, const unsigned int value);

/* update given key-value pairs if key exists, o/w insert it */
void DictInsertOrUpdate(Dict d, const unsigned int key, const unsigned int value);

/* return the most recently inserted value associated with a key */
/* or 0 if no matching key is present */
const unsigned int DictSearch(Dict, const unsigned int key);

/* delete the most recently inserted record with the given key */
/* if there is no such record, has no effect */
void DictDelete(Dict, const unsigned int key);
