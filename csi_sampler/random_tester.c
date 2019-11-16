#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <sys/time.h>

#define NOF_DOCS 5000000

unsigned int rand_interval(unsigned int min, unsigned int max) {
    int r;
    const unsigned int range = 1 + max - min;
    const unsigned int buckets = RAND_MAX / range;
    const unsigned int limit = buckets * range;

    /* Create equal size buckets all in a row, then fire randomly towards
     * the buckets until you land in one of them. All buckets are equally
     * likely. If you land off the end of the line of buckets, try again. */
    do
    {
        r = rand();
    } while (r >= limit);

    return min + (r / buckets);
}

// Assumes 0 <= max <= RAND_MAX
// Returns in the closed interval [0, max]
long random_at_most(long max) {
  unsigned long
    // max <= RAND_MAX < ULONG_MAX, so this is okay.
    num_bins = (unsigned long) max + 1,
    num_rand = (unsigned long) RAND_MAX + 1,
    bin_size = num_rand / num_bins,
    defect   = num_rand % num_bins;

  long x;
  do {
   x = random();
  }
  // This is carefully written not to overflow
  while (num_rand - defect <= (unsigned long)x);

  // Truncated division is intentional
  return x/bin_size;
}

int main(int argc, char** argv) {
    char marks[NOF_DOCS] = "";
    int count = 0;

    struct timeval tm;
    gettimeofday(&tm, NULL);
    srandom(tm.tv_sec + tm.tv_usec * 1000000ul);

    while (count < NOF_DOCS) {

        long number = random_at_most(NOF_DOCS-1);
        // long number = rand_interval(0, NOF_DOCS-1);
        if (marks[number] == '.') {
            continue;
        }

        marks[number] = '.';
        count++;

        if (count % 10000 == 0) {
            printf("%ld_", number);
            fflush(stdout);
        }
    }

    printf("\n");
    return 0;
}
