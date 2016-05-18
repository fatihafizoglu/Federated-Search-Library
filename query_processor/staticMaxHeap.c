#include <stdio.h>
#include <stdlib.h>
#include "staticMaxHeap.h"

void createMaxHeap(struct staticMaxHeapStruct *maxHeap, int maxSize) {
    maxHeap->itemCount = 0;
    maxHeap->maxSize = maxSize;
    maxHeap->items = (Result *) calloc(maxSize, sizeof(Result));
}

void heapifyMaxHeap(struct staticMaxHeapStruct *maxHeap, int current) {
    Result tmpItem;
    int left, right, largest;

    left = 2*current+1;
    right = left+1;

    if (left < maxHeap->itemCount && maxHeap->items[left].sim_rank > maxHeap->items[current].sim_rank)
        largest = left;
    else
        largest = current;

    if (right < maxHeap->itemCount && maxHeap->items[right].sim_rank > maxHeap->items[largest].sim_rank)
        largest = right;

    if (largest != current) {
        tmpItem = maxHeap->items[current];
        maxHeap->items[current] = maxHeap->items[largest];
        maxHeap->items[largest] = tmpItem;
        heapifyMaxHeap(maxHeap, largest);
    }
}

void buildMaxHeap(struct staticMaxHeapStruct *maxHeap) {
    int i;

    for (i = maxHeap->itemCount/2-1; i >= 0; i--)
        heapifyMaxHeap(maxHeap, i);
}

int insertMaxHeap(struct staticMaxHeapStruct *maxHeap, int docId, double score) {
    int current;

    if (maxHeap->itemCount == maxHeap->maxSize)
        return -1;

    current = maxHeap->itemCount;

    while (current > 0 && maxHeap->items[(current-1)/2].sim_rank < score) {
        maxHeap->items[current] = maxHeap->items[(current-1)/2];
        current = (current-1)/2;
    }

    maxHeap->items[current].doc_index = docId;
    maxHeap->items[current].sim_rank = score;
    maxHeap->itemCount++;

    return 0;
}

int extractMaxHeap(struct staticMaxHeapStruct *maxHeap, int *docId, double *score) {
    if (!maxHeap->itemCount)
        return -1;

    *docId = maxHeap->items[0].doc_index;
    *score = maxHeap->items[0].sim_rank;
    maxHeap->items[0] = maxHeap->items[maxHeap->itemCount-1];
    maxHeap->itemCount--;
    heapifyMaxHeap(maxHeap, 0);

    return 0;
}

int queryMaxMaxHeap(struct staticMaxHeapStruct *maxHeap, int *docId, double *score) {
    if (!maxHeap->itemCount)
        return -1;

    *docId = maxHeap->items[0].doc_index;
    *score = maxHeap->items[0].sim_rank;

    return 0;
}

void printMaxHeap(struct staticMaxHeapStruct *maxHeap) {
    int i;

    if (!maxHeap->itemCount) {
        printf("empty\n");
        return;
    }

    for (i = 0; i < maxHeap->itemCount; i++)
        printf("docId %d with score %.3f\n", maxHeap->items[i].doc_index, maxHeap->items[i].sim_rank);
}

void freeMaxHeap(struct staticMaxHeapStruct *maxHeap) {
    maxHeap->itemCount = 0;
    free(maxHeap->items);
}
