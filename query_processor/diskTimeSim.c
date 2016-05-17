#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <math.h>

double RAND_ACCESS_TIME = 8.6; //ms (ssek + latancy etc)
double PAGE_SIZE = 512;
double BLOCK_TRANS_TIME = 0.014; //ms
double total_random_access = 0;
double total_sequential_access = 0;
double total_disk_time = 0;

void uncompressed_DISK_TIME(int O) {
    total_random_access += RAND_ACCESS_TIME;
    total_sequential_access += ceil((O * 2 *sizeof(int))/PAGE_SIZE) * BLOCK_TRANS_TIME;
    total_disk_time += RAND_ACCESS_TIME + ceil((O * 2 *sizeof(int))/PAGE_SIZE) * BLOCK_TRANS_TIME;
}

void compressed_DISK_TIME(int O) {
    total_random_access += RAND_ACCESS_TIME;
    total_sequential_access += ceil(O/PAGE_SIZE) * BLOCK_TRANS_TIME;
    total_disk_time += RAND_ACCESS_TIME + ceil(O/PAGE_SIZE) * BLOCK_TRANS_TIME;
}
