#include "Allocator.h"

/*
 * Reads merged wordlist file and fills terms.
 * Returns number of terms loaded.
 */
int loadTerms () {
    FILE *fp;
    int terms_count = 0;
    
    if (!(fp = fopen(config->wordlist_path,"r"))) {
    	perror(error_messages[FILE_NOT_FOUND]);
    	exit(EXIT_FAILURE);
    }

	while (!feof(fp)) {
		char token[1];
		//Term *term = *terms + sizeof(Term) * terms_count;
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
 * Returns 0 if success, -1 otherwise.
 */
int initAllocator (Conf *conf) {
    if (!conf)
    	return EMPTY_CONFIG_DATA;
    
    config = conf;
    terms = malloc(config->number_of_terms * sizeof(Term));
    documents = malloc(config->number_of_documents * sizeof(Document));
    
    if (terms == NULL || documents == NULL)
        return INIT_NULL_MALLOC;
    
    return SUCCESS;
}

// TODO: Error-lar ve error mesajlari nasil handle edilecek?
// TODO: const char* header'da static olmadan neden tanimlanamiyor. (multiple definition)
