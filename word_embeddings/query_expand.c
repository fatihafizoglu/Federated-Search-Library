#include "word_embeddings.h"


int main (int argc, char** argv) {
    int q_index = 0;
    int ret = 0;

    ret = load_dictionary();
    if (ret != GLOVE_DICT_SIZE) {
        printf("load_dictionary failed ret:%d\n", ret);
        exit(1);
    }

    real_nof_queries = load_queries();
    if (real_nof_queries <= 0) {
        printf("load_queries failed: %d\n", real_nof_queries);
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
    return ret;
}
