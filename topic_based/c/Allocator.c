#include "Allocator.h"

/*
 * Should be called after any operation ends.
 * Checks program state and act according to this state.
 */
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

/*
 * Returns document vectors file path.
 */
char *getDocumentVectorsFilepath (unsigned int doc_id) {
    char *document_vectors_filepath;
    short file_index;

    if ((1 <= doc_id) && (doc_id < 5000000))
        file_index = 0;
    else if ((5000000 <= doc_id) && (doc_id < 10000000))
        file_index = 1;
    else if ((10000000 <= doc_id) && (doc_id < 15000000))
        file_index = 2;
    else if ((15000000 <= doc_id) && (doc_id  < 20000000))
        file_index = 3;
    else if ((20000000 <= doc_id) && (doc_id < 25000000))
        file_index = 4;
    else if ((25000000 <= doc_id) && (doc_id  < 30000000))
        file_index = 5;
    else if ((30000000 <= doc_id) && (doc_id < 35000000))
        file_index = 6;
    else if ((35000000 <= doc_id) && (doc_id  < 40000000))
        file_index = 7;
    else if ((40000000 <= doc_id) && (doc_id  < 45000000))
        file_index = 8;
    else if ((45000000 <= doc_id) && (doc_id  < 50000000))
        file_index = 9;
    else if ((50000000 <= doc_id) && (doc_id  < 50220539))
        file_index = 10;

    size_t len1 = strlen(config->document_vectors_folder_path);
    size_t len2 = strlen(document_vectors_files[file_index]);
    document_vectors_filepath = malloc(len1+len2+1+1);
    memcpy(document_vectors_filepath, config->document_vectors_folder_path, len1);
    memcpy(document_vectors_filepath+len1, "/", 1);
    memcpy(document_vectors_filepath+len1+1, document_vectors_files[file_index], len2+1);

    return document_vectors_filepath;
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

    FILE *fp;
    TermVectors term_vectors;
    char *document_vectors_filepath = getDocumentVectorsFilepath(document->doc_id);
    long term_vectors_alloc_size = (long)document->uterm_count * TERM_ID_TF_PAIR_SIZE;

    if (!(term_vectors = malloc(term_vectors_alloc_size))) {
        state = COULD_NOT_ALLOCATE_TERM_VECTORS;
        return NULL;
    }

    // TODO: butun dosyalari basta bir kere acmak, her sey bitince kapamak daha
    // dogru olabilir mi?
    if (!(fp = fopen(document_vectors_filepath, "r"))) {
        state = COULD_NOT_OPEN_DOCUMENT_VECTORS_FILE;
        return NULL;
    }

    // ...




    free(document_vectors_filepath);
    state = SUCCESS;
    return term_vectors;
}

/*
 * Reads documents info file and fills documents.
 * Returns number of documents loaded, calculate
 * offset wrt unique term counts.
 *
 * Note that: Document vectors seperated as: [#document id - #document id]
 * File 1 : [1          - 4 999 999]
 * File 2 : [5 000 000  - 9 999 999]
 * File 3 : [10 000 000 - 14 999 999]
 * ...
 * File 10 : [45 000 000 - 49 999 999]
 * File 11 : [50 000 000 - 50 220 538]
 */
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

/*
 * Reads merged wordlist file and fills terms.
 * Returns number of terms loaded.
 */
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

/*
 * Takes paths of a merged wordlist file, document info file and
 * folder containing document vectors files (i.e. dvec.bin).
 * Returns 0 if success, -1 otherwise and sets state.
 */
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

/*
 * TODO: const char* header'da static olmadan neden tanimlanamiyor. (multiple definition)
 */
