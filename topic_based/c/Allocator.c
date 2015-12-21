#include "Allocator.h"

/*
 * Returns document vectors file.
 */
static FILE* getDocumentVectorsFile (unsigned int doc_id) {
    short file_index = doc_id / 5000000;

    return document_vectors_files[file_index];
}

/*
 * Returns term vectors of a given document.
 * Term vectors are lists of <term id, term frequency> pairs.
 */
TermVectors getTermVectors (Document* document) {
    if (!document) {
        state = NULL_DOCUMENT;
        return NULL;
    }

    TermVectors term_vectors;
    FILE *fp = getDocumentVectorsFile(document->doc_id);

    long term_vectors_alloc_size = (long)document->uterm_count * TERM_ID_TF_PAIR_SIZE;
    if (!(term_vectors = malloc(term_vectors_alloc_size))) {
        state = COULD_NOT_ALLOCATE_TERM_VECTORS;
        return NULL;
    }
    size_t read_length;
    off_t address = (long)document->offset * TERM_ID_TF_PAIR_SIZE;
    fseeko(fp, address, SEEK_SET);
    read_length = fread(term_vectors, TERM_ID_TF_PAIR_SIZE, (size_t)document->uterm_count, fp);
    if (read_length != (size_t)document->uterm_count) {
        state = COULD_NOT_READ_TERM_VECTORS_PROPERLY;
        return term_vectors;
    }

    state = SUCCESS;
    return term_vectors;
}

/*
 * Adds document to cluster's temporary dictionary.
 */
static void addDocumentToCluster(Cluster* cluster, Document* document) {
    int i;
    TermVectors document_term_vectors = getTermVectors(document);
    for (i = 0; i < document->uterm_count; i++) {
        DictIncreaseOrInsert(cluster->new_dictionary, document_term_vectors[i].term_id, document_term_vectors[i].term_frequency);
        cluster->new_term_count += document_term_vectors[i].term_frequency;
    }
}

/*
 * Returns the pointer to the document that has given document id.
 */
Document *getDocument(unsigned int doc_id) {
    return &documents[doc_id-1];
}

/*
 * Comparison function for quick sort.
 */
static int cmpfunc (const void * a, const void * b) {
   return ( *(int*)a - *(int*)b );
}

/*
 * Returns values in the range [min, max], where
 * max >= min and 1+max-min < RAND_MAX
 */
static unsigned int
rand_interval(unsigned int min, unsigned int max) {
    int r;
    const unsigned int range = 1 + max - min;
    const unsigned int buckets = RAND_MAX / range;
    const unsigned int limit = buckets * range;

    /* Create equal size buckets all in a row, then fire randomly towards
     * the buckets until you land in one of them. All buckets are equally
     * likely. If you land off the end of the line of buckets, try again. */
    do {
        r = rand();
    } while (r >= limit);

    return min + (r / buckets);
}

/*
 * Puts sample_count random integer to sample_indeces in range (1, max).
 */
static void
randomSample (unsigned int *samples, unsigned int sample_count, unsigned int max) {
    for (int i = 0; i < sample_count; i++) {
        samples[i] = rand_interval(1, max);
        //samples[i] = rand_interval(15000000, 19999999);
    }
}

void swapDictionary () {
    int i;

    for (i = 0; i < NUMBER_OF_CLUSTERS; i++) {
        clusters[i].term_count = clusters[i].new_term_count;
        clusters[i].new_term_count = 0;

        DictDestroy(clusters[i].dictionary);
        clusters[i].dictionary = clusters[i].new_dictionary;
        clusters[i].new_dictionary = DictCreate();
    }

    merged_cluster.term_count = merged_cluster.new_term_count;
    merged_cluster.new_term_count = 0;

    DictDestroy(merged_cluster.dictionary);
    merged_cluster.dictionary = merged_cluster.new_dictionary;
    merged_cluster.new_dictionary = DictCreate();
}

void kMeans() {
    int i, j, k;
    unsigned int sample_count = config->number_of_documents * PERCENTAGE_OF_SAMPLES;

    for (i = 0; i < sample_count; i++) {
        Document *document = getDocument(sample_doc_ids[i]);
        TermVectors document_term_vectors = getTermVectors(document);
        double max_similarity_value = 0.0;
        unsigned int most_similar_cluster_index = 0;

        for (j = 0; j < NUMBER_OF_CLUSTERS; j++) {
            Dict cluster_dictionary = clusters[j].dictionary;
            double similarity_value = 0.0;

            for (k = 0; k < document->uterm_count; k++) {
                TermVector tv = document_term_vectors[k];
                if (DictSearch(cluster_dictionary, tv.term_id) != 0) {
                    double pciw = (double)DictSearch(cluster_dictionary, tv.term_id) / clusters[j].term_count;
                    double pbw = (double)DictSearch(merged_cluster.dictionary, tv.term_id) / merged_cluster.term_count;
                    double pdw = ((1.0 - LAMBDA) * (double)tv.term_frequency / document->term_count) + LAMBDA * pbw;
                    similarity_value += (pciw * log10(pdw/(LAMBDA*pbw))) + (pdw * log10(pciw/(LAMBDA*pbw)));
                }
            }

            if (similarity_value > max_similarity_value) {
                max_similarity_value = similarity_value;
                most_similar_cluster_index = j;
            }
        }

        addDocumentToCluster(&clusters[most_similar_cluster_index], document);
        addDocumentToCluster(&merged_cluster, document);
    }
}

void initializeKMeans () {
    int i;

    // TODO: select first n documents randomly from sampled documents.
    for (i = 0; i < NUMBER_OF_CLUSTERS; i++) {
        addDocumentToCluster(&clusters[i], getDocument(sample_doc_ids[i]));
        addDocumentToCluster(&merged_cluster, getDocument(sample_doc_ids[i]));
    }
}

int sampleDocuments() {
    unsigned int sample_count = config->number_of_documents * PERCENTAGE_OF_SAMPLES;
    if (!(sample_doc_ids = malloc(sample_count*sizeof(int)))) {
        state = COULD_NOT_ALLOCATE_DOCUMENT_IDS;
        return -1;
    }

    randomSample(sample_doc_ids, sample_count, config->number_of_documents);
    qsort(sample_doc_ids, sample_count, sizeof(int), cmpfunc);

    state = SUCCESS;
    return 0;
}

int initClusters () {
    if (!(clusters = malloc(NUMBER_OF_CLUSTERS * sizeof(Cluster)))) {
        state = COULD_NOT_ALLOCATE_CLUSTERS;
        return -1;
    }

    /* Init merged cluster. */
    merged_cluster.term_count = 0;
    merged_cluster.new_term_count = 0;
    merged_cluster.dictionary = DictCreate();
    merged_cluster.new_dictionary = DictCreate();
    /*merged_cluster.document_count = 0;
    if (!(merged_cluster.document_ids = malloc(INITIAL_SIZE * sizeof(int)))) {
        state = COULD_NOT_ALLOCATE_CLUSTER_DOCUMENT_IDS;
        return -1;
    }*/

    /* Init clusters. */
    for (size_t i = 0; i < NUMBER_OF_CLUSTERS; i++) {
        clusters[i].term_count = 0;
        clusters[i].new_term_count = 0;
        clusters[i].dictionary = DictCreate();
        clusters[i].new_dictionary = DictCreate();
        /*clusters[i].document_count = 0;
        if (!(clusters[i].document_ids = malloc(INITIAL_SIZE * sizeof(int)))) {
            state = COULD_NOT_ALLOCATE_CLUSTER_DOCUMENT_IDS;
            return -1;
        }*/
    }

    state = SUCCESS;
    return 0;
}

int loadDocuments () {
    FILE *fp;
    unsigned int documents_count = 0;
    unsigned int inside_file_offset = 0;

    if (!(fp = fopen(config->document_info_path, "r"))) {
        state = COULD_NOT_OPEN_DOCUMENT_INFO;
        return 0;
    }

    while (!feof(fp)) {
        fscanf (fp, "%u %u %u\n", &(documents[documents_count].doc_id),
                                  &(documents[documents_count].uterm_count),
                                  &(documents[documents_count].term_count));

        documents[documents_count].offset = inside_file_offset;
        inside_file_offset += documents[documents_count].uterm_count;
        if (documents_count % 5000000 == 4999998)
                inside_file_offset = 0;

        documents_count++;
    }

    fclose(fp);
    state = SUCCESS;
    return documents_count;
}

int loadTerms () {
    FILE *fp;
    unsigned int terms_count = 0;

    if (!(fp = fopen(config->wordlist_path, "r"))) {
        state = COULD_NOT_OPEN_WORDLIST;
        return 0;
    }

    while (!feof(fp)) {
        fscanf (fp, "%s %u %lf\n", (terms[terms_count].token),
                                    &(terms[terms_count].total_count),
                                    &(terms[terms_count].cfc_weight));
        terms[terms_count].term_id = terms_count;
        terms_count++;
    }

    fclose(fp);
    state = SUCCESS;
    return terms_count;
}

void closeDocumentVectorsFiles () {
    int i;
    for (i = 0; i < NUMBER_OF_DOCUMENT_VECTORS_FILES; i++)
        fclose(document_vectors_files[i]);

    free(document_vectors_files);
    state = SUCCESS;
}

/*
 * Returns document vectors file path as a string.
 */
static char *getDocumentVectorsFilepath (unsigned int index) {
    char *document_vectors_filepath;

    size_t len1 = strlen(config->document_vectors_folder_path);
    size_t len2 = strlen(document_vectors_file_names[index]);
    document_vectors_filepath = malloc(len1+len2+1+1);
    memcpy(document_vectors_filepath, config->document_vectors_folder_path, len1);
    memcpy(document_vectors_filepath+len1, "/", 1);
    memcpy(document_vectors_filepath+len1+1, document_vectors_file_names[index], len2+1);

    return document_vectors_filepath;
}

int openDocumentVectorsFiles () {
    int i;
    document_vectors_files = malloc(sizeof(FILE *) * NUMBER_OF_DOCUMENT_VECTORS_FILES);

    for (i = 0; i < NUMBER_OF_DOCUMENT_VECTORS_FILES; i++) {
        char *document_vectors_filepath = getDocumentVectorsFilepath(i);
        if (!(document_vectors_files[i] = fopen(document_vectors_filepath, "r"))) {
            state = COULD_NOT_OPEN_DOCUMENT_VECTORS_FILE;
            return -1;
        }

        free(document_vectors_filepath);
    }

    state = SUCCESS;
    return 0;
}

void actState () {
    printf("%s\n", state_messages[state]);

    switch (state) {
        case SUCCESS:
            break;
        case EMPTY_CONFIG_DATA:
        case COULD_NOT_ALLOCATE_TERMS:
        case COULD_NOT_ALLOCATE_DOCUMENTS:
        case COULD_NOT_OPEN_WORDLIST:
        case COULD_NOT_OPEN_DOCUMENT_INFO:
        case COULD_NOT_ALLOCATE_TERM_VECTORS:
        default:
            exit(EXIT_FAILURE);
    }
}

int initAllocator (Conf *conf) {
    if (!conf) {
        state = EMPTY_CONFIG_DATA;
        return -1;
    }

    config = conf;

    long term_alloc_size = config->number_of_terms * sizeof(Term);
    long documents_alloc_size = config->number_of_documents * sizeof(Document);

    if (!(terms = malloc(term_alloc_size))) {
        state = COULD_NOT_ALLOCATE_TERMS;
        return -1;
    } else if (!(documents = malloc(documents_alloc_size))) {
        state = COULD_NOT_ALLOCATE_DOCUMENTS;
        return -1;
    }

    state = SUCCESS;
    return 0;
}
