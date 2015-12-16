#include "messages.h"

/*
 * Note from EmreCan:
 * const used to avoid 'defined but not used' warning.
 */

char * firstMessage = "dummy message";

char* const state_messages[] = {
	(char*)&firstMessage,
    "Successful operation.",
    "Empty configuration data.",
    "Could not allocate Terms.",
    "Could not allocate Documents.",
    "Could not open merged wordlist."
};
