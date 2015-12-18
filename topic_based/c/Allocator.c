#include "Allocator.h"

/*
 * Should be called after any operation ends.
 * Checks program state and act according to this state.
 */
void actstate () {
    printf("%s\n", state_messages[state]);

    if (state == COULD_NOT_ALLOCATE_TERMS ||
        state == COULD_NOT_ALLOCATE_DOCUMENTS ||
        state == EMPTY_CONFIG_DATA ||
        state == COULD_NOT_OPEN_WORDLIST ||
        state == COULD_NOT_OPEN_DOCUMENT_INFO) {

        exit(EXIT_FAILURE);
    }
}

/*
 * Reads merged wordlist file and fills terms.
 * Returns number of terms loaded.
 */
int loadTerms () {
    FILE *fp;
    int terms_count = 0;

    if (!(fp = fopen(config->wordlist_path,"r"))) {
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
    return terms_count;
}

/*
 * Reads documents info file and fills documents.
 * Returns number of documents loaded.
 */
int loadDocuments () {
    FILE *fp;
    unsigned int documents_count = 0;
    unsigned int inside_file_offset = 0;

    if (!(fp = fopen(config->document_info_path,"r"))) {
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
    return documents_count;
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
