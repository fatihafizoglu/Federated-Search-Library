#include "word_embeddings.h"


int main (int argc, char** argv) {
    int q_index = 0;

    if (load_dictionary() != GLOVE_DICT_SIZE) {
        printf("load_dictionary failed\n");
        exit(1);
    }

    if (load_queries() <= 0) {
        printf("load_queries failed\n");
        exit(1);
    }

    if (!(qout_fp = fopen(QUERIES_PATH_OUT, "w"))) {
        printf("fopen failed: %s\n", QUERIES_PATH_OUT);
        exit(1);
    }

    if (!(qdout_fp = fopen(QUERIES_DIVED_PATH_OUT, "w"))) {
        printf("fopen failed: %s\n", QUERIES_DIVED_PATH_OUT);
        exit(1);
    }

    // dictionary and query word embeddings are ready.
    // time to find words to expand
    // for a query, calculate cosine sim
    for (q_index = 0; q_index < real_nof_queries; q_index++) {
        // for each words in dictionary
        // then find and write top NOF_WORDS_TO_EXPAND
        // together with existing query words
        if (expand_query(q_index) != 0) {
            printf("expand_query failed: %d\n", q_index);
            exit(1);
        }
    }

    fclose(qout_fp);
    fclose(qdout_fp);
    return 0;
}
