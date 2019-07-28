#define NOSTOPWORD 419
#define MAX_TUPLE_LENGTH 300000 // wrt. the tuple definition below...
#define WORD_NO 164000000  // SORUN: Bu  ne kadar buyuk olabilir? I hope it will work
#define TOKEN_NO  20000 // max no of tokens in a field: now it is token no at each doc line
#define TOKEN_SIZE 21 // max length of a token
#define DOC_SIZE 600000
#define DOC_NUM 50220539 /* exact number: 50220538 */ // get 1 more than exact value, so is word no
#define BUFFERSIZE DOC_NUM
#define QUERY_NO 1

#define DETAILED_LOG 0

#define TFIDF 1 // bnm orijinal measure, doc term wegits: tf ve IDF, query term: augmentd tf ve IDF, document vector length'e bolunur.
#define MF8 2
#define BM25 3
#define CARMEL_SMART 4
#define DIRICHLET 5

#define SIM_MEASURE BM25
#define RUN_NO 1

#define QSIZE 3000
#define BEST_DOCS 200
#define TOP_N BEST_DOCS

typedef struct inv_ent {
    int doc_id;
    int weight;
} InvEntry;

double BM25_K1_CONSTANT = 1.2;
double BM25_B_CONSTANT = 0.5;
int DIRICHLET_CONSTANT = 2000; // from ECIR 2012 diversty best paer thing! 1500; //from zettair

double doc_lengths[DOC_NUM];
