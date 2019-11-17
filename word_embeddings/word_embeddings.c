#include "word_embeddings.h"

// double dotProduct (TermVectors v1, int len1, double *tf_idf1, TermVectors v2, int len2, double *tf_idf2) {
//     double ret = 0.0;
//     int i = 0, j = 0;
//
//     while (i < len1 && j < len2) {
//         if (v1[i].term_id == v2[j].term_id) {
//             ret += tf_idf1[i] * tf_idf2[j];
//             i++;
//             j++;
//         } else if (v1[i].term_id < v2[j].term_id)
//             i++;
//         else if (v1[i].term_id > v2[j].term_id)
//             j++;
//     }
//
//     return ret;
// }
//
// double getVectorLength (int length, double *tf_idf) {
//     double ret = 0.0;
//     int i;
//
//     for (i = 0; i < length; i++)
//         ret += tf_idf[i] * tf_idf[i];
//
//     ret = sqrt(ret);
//     return ret;
// }
//
// double cosineSimilarity (int doc1_id, int doc2_id) {
//     double ret = 0.0;
//     Document *doc1 = getDocument(doc1_id);
//     Document *doc2 = getDocument(doc2_id);
//     double tf_idf1[doc1->uterm_count];
//     double tf_idf2[doc2->uterm_count];
//     TermVectors doc1_term_vectors = getTermVectors(doc1, tf_idf1);
//     TermVectors doc2_term_vectors = getTermVectors(doc2, tf_idf2);
//
//     ret = dotProduct(doc1_term_vectors, doc1->uterm_count, tf_idf1,
//                      doc2_term_vectors, doc2->uterm_count, tf_idf2);
//
//     ret = ret / (getVectorLength(doc1->uterm_count, tf_idf1) * getVectorLength(doc2->uterm_count, tf_idf2));
//
//     free(doc1_term_vectors);
//     free(doc2_term_vectors);
//     return ret;
// }

void init_queries () {
    int i, j;
    for (i = 0; i < MAX_NOF_QUERIES; i++) {
        strcpy(queries[i].word, "");
        for (j = 0; j < GLOVE_VECTOR_SIZE; j++) {
            queries[i].vector[j] = 0.0;
        }
    }
}

void init_dictionary () {
    int i, j;
    for (i = 0; i < GLOVE_DICT_SIZE; i++) {
        strcpy(dictionary[i].word, "");
        for (j = 0; j < GLOVE_VECTOR_SIZE; j++) {
            dictionary[i].vector[j] = 0.0;
        }
    }
}

int load_dictionary () {
    init_dictionary();

    // XXXread glove datafile
    // XXXload dictionary

    return 0;
}

int load_queries () {
    int nof_queries = 0;
    init_queries();

    // XXXread query file

    // XXXfor each word in query find we
    // and then birlestir them

    // XXXthen store birlestirilmis we in queries we

    return nof_queries;
}

double cosine_similarity (We we1, We we2) {
    double score = 0.0;

    // XXXboth we has same #nof vector compute score

    return score;
}

int write_all () {

    return 0;
}

int expand_query (int q_index) {
    double *query_word_similarities;
    int w_index = 0;

    query_word_similarities = malloc(GLOVE_DICT_SIZE * sizeof(double));
    if (!query_word_similarities) {
        printf("query_word_similarities malloc failed\n");
        exit(1);
    }
    memset(query_word_similarities, 0, GLOVE_DICT_SIZE * sizeof(double));
    printf("Check %lf, %lf\n", query_word_similarities[0],
        query_word_similarities[50]);

    for (w_index = 0; w_index < GLOVE_DICT_SIZE; w_index++) {
        query_word_similarities[w_index] =
            cosine_similarity(queries[q_index], dictionary[w_index]);
    }

    // XXXfirst find the top k scores indexes from query_word_similarities
    // XXXthen, write the orig query to fp w/new k words from dictionary

    // XXX1 Change algo w/diversified expansion

    free(query_word_similarities);
    return 0;
}
