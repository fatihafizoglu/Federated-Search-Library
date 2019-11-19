#ifndef STATIC_MAX_HEAP
#define STATIC_MAX_HEAP

typedef struct res {
    int doc_index;
    double sim_rank;
} Result, *RP;

struct staticMaxHeapStruct {
    Result *items;
    int itemCount;
    int maxSize;
};

void createMaxHeap(struct staticMaxHeapStruct *maxHeap, int maxSize);
void heapifyMaxHeap(struct staticMaxHeapStruct *maxHeap, int current);
void buildMaxHeap(struct staticMaxHeapStruct *maxHeap);
int insertMaxHeap(struct staticMaxHeapStruct *maxHeap, int docId, double score);
int extractMaxHeap(struct staticMaxHeapStruct *maxHeap, int *docId, double *score);
int queryMaxMaxHeap(struct staticMaxHeapStruct *maxHeap, int *docId, double *score);
void printMaxHeap(struct staticMaxHeapStruct *maxHeap);
void freeMaxHeap(struct staticMaxHeapStruct *maxHeap);

#endif
