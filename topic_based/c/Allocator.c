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
        state == COULD_NOT_OPEN_WORDLIST) {

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
        actstate();
    }

    while (!feof(fp)) {
        char token[5];
        /* Term *term = *terms + sizeof(Term) * terms_count; */
        Term *term = &(terms[terms_count]);
        fscanf (fp, "%s %d %lf\n", token, &(term->total_count), &(term->cfc_weight));
#ifdef DEBUG
        term->token = token;
        term->term_id = terms_count + 1;
#endif
        terms_count++;
    }

    fclose(fp);
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

    if (!(terms = malloc(config->number_of_terms * sizeof(Term)))) {
        state = COULD_NOT_ALLOCATE_TERMS;
        return -1;
    } else if (!(documents = malloc(config->number_of_documents * sizeof(Document)))) {
        state = COULD_NOT_ALLOCATE_DOCUMENTS;
        return -1;
    }


    printf("%d\n", sizeof(Term)); // 16 - 24
    printf("%d\n", sizeof(Document)); // 24 - 24

    state = SUCCESS;
    return 0;
}

/*
 * TODO: Error-lar ve error mesajlari nasil handle edilecek?
 * TODO: const char* header'da static olmadan neden tanimlanamiyor. (multiple definition)
 */
