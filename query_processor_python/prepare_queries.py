import numpy as np
import os
import time

print "Script Started at: " + time.strftime('%X %x')

DUMP_LOCATION = "/home/fatihafizoglu/git/MS/document-allocation-policies/query_processor_python/saved/"
WORD_NO = 164000000
STOPWORD_NO = 423

stopword_list_file_path = "/media/fatihafizoglu/LenovoMS/MS/stopword.lst"
stopword_list_words = None

query_file_path = "/home/fatihafizoglu/Desktop/trec_eval/TREC_2009_WebTrack/REQUIRED/queries_stopword_eleminated.txt"
query_lengths = []
query_terms = {}

word_list_file_path = "/media/fatihafizoglu/LenovoMS/MS/Index/merged_wordlist.txt"
word_list_file_path_topic_based_clusters_csi = "/media/fatihafizoglu/LenovoMS/MS/Index/merged_wordlist_TopicBasedClusters_100_2_CSI.txt"
word_list_file_path_random_based_clusters_csi = "/media/fatihafizoglu/LenovoMS/MS/Index/merged_wordlist_RandomBasedClusters_100_CSI.txt"
word_list_words = None
word_list_occurance_in_docs = None
word_list_cfc_weights = None
word_list_disk_addresses = None

index_main_file_path = "/media/fatihafizoglu/LenovoMS/MS/Index/merged_entry.txt"
index_topic_csi_file_path = "/media/fatihafizoglu/LenovoMS/MS/Index/merged_entry_TopicBasedClusters_100_2_CSI.txt"

def read_stopword_list(path):
    global stopword_list_words

    stopword_list_words = np.zeros(STOPWORD_NO, dtype="S70")

    stopword_list_words_dump_path = os.path.join(DUMP_LOCATION, "stopword_list_words.npy")

    if os.path.exists(stopword_list_words_dump_path):
        print "INFO: Loading " + path
        stopword_list_words = np.load(stopword_list_words_dump_path)
        print "INFO: Finished " + path
    else:
        print "INFO: Reading " + path
        stopword_list_file = open(stopword_list_file_path, 'r')

        line_index = 1
        for line in stopword_list_file:
            line_splitted = line.split()
            if len(line_splitted):
                stopword_list_words[line_index] = line_splitted[0].lower()
            line_index += 1

        stopword_list_file.close()
        print "INFO: Finished " + path

        np.save(stopword_list_words_dump_path, stopword_list_words)

def read_queries(path, remove_stopwords):
    global query_lengths
    global query_terms

    print "INFO: Reading " + path
    query_file = open(path, 'r')

    line_index = 1
    for line in query_file:
        line_splitted = line.split()
        line_splitted.sort()

        query_terms[line_index] = []
        total_term_occurance_in_query = 0
        for i in range(len(line_splitted)):
            if remove_stopwords:
                spam_index = stopword_list_words.searchsorted(line_splitted[i])
                if stopword_list_words[spam_index] == line_splitted[i]:
                    continue
            if len(query_terms[line_index]) == 0:
                query_terms[line_index].append({
                    "word" : line_splitted[i],
                    "occurance_in_query" : 1,
                    })
            else:
                if query_terms[line_index][-1]["word"] == line_splitted[i]:
                    query_terms[line_index][-1]["occurance_in_query"] += 1
                else:
                    query_terms[line_index].append({
                        "word" : line_splitted[i],
                        "occurance_in_query" : 1,
                        })
            total_term_occurance_in_query += 1
        query_lengths.append(total_term_occurance_in_query)
        line_index += 1

    query_file.close()
    print "INFO: Finished " + path

def read_word_list(path, index_type):
    global word_list_words
    global word_list_occurance_in_docs
    global word_list_cfc_weights
    global word_list_disk_addresses

    word_list_words = np.zeros(WORD_NO, dtype="S70")
    word_list_occurance_in_docs = np.empty(WORD_NO, dtype=np.uint32)
    word_list_cfc_weights = np.empty(WORD_NO, dtype=np.float)
    word_list_disk_addresses = np.empty(WORD_NO, dtype=np.uint64)

    word_list_words_dump_path = os.path.join(DUMP_LOCATION, index_type, "word_list_words.npy")
    word_list_occurance_in_docs_dump_path = os.path.join(DUMP_LOCATION, index_type, "word_list_occurance_in_docs.npy")
    word_list_cfc_weights_dump_path = os.path.join(DUMP_LOCATION, index_type, "word_list_cfc_weights.npy")
    word_list_disk_adrdresses_dump_path = os.path.join(DUMP_LOCATION, index_type, "word_list_disk_addresses.npy")

    if (os.path.exists(word_list_words_dump_path) and os.path.exists(word_list_occurance_in_docs_dump_path) and os.path.exists(word_list_cfc_weights_dump_path) and os.path.exists(word_list_disk_adrdresses_dump_path)):
        print "INFO: Loading " + path
        word_list_words = np.load(word_list_words_dump_path)
        word_list_occurance_in_docs = np.load(word_list_occurance_in_docs_dump_path)
        word_list_cfc_weights = np.load(word_list_cfc_weights_dump_path)
        word_list_disk_addresses = np.load(word_list_disk_adrdresses_dump_path)
        print "INFO: Finished " + path
    else:
        print "INFO: Reading " + path
        word_list_file = open(path, 'r')

        line_index = 1
        for line in word_list_file:
            line_splitted = line.split()
            word_list_words[line_index] = line_splitted[0].lower()
            word_list_occurance_in_docs[line_index] = int(line_splitted[1])
            word_list_cfc_weights[line_index] = float(line_splitted[2])
            if line_index > 1:
                word_list_disk_addresses[line_index] = word_list_disk_addresses[line_index - 1] + word_list_occurance_in_docs[line_index - 1]
            else:
                word_list_disk_addresses[line_index] = 0
            line_index += 1

        word_list_file.close()
        print "INFO: Finished " + path

        np.save(word_list_words_dump_path, word_list_words)
        np.save(word_list_occurance_in_docs_dump_path, word_list_occurance_in_docs)
        np.save(word_list_cfc_weights_dump_path, word_list_cfc_weights)
        np.save(word_list_disk_adrdresses_dump_path, word_list_disk_addresses)

def get_reletad_word_statistics_from_word_list(index_type):
    global query_terms
    global word_list_words
    global word_list_occurance_in_docs
    global word_list_disk_addresses
    global word_list_cfc_weights

    for query_index, term_dict_array in query_terms.iteritems():
        for term_dict in term_dict_array:
            word_list_index = word_list_words.searchsorted(term_dict["word"])
            if word_list_words[word_list_index] == term_dict["word"]:
                term_dict[index_type] = {}
                term_dict[index_type]["occurance_in_docs"] = word_list_occurance_in_docs[word_list_index]
                term_dict[index_type]["disk_address"] = word_list_disk_addresses[word_list_index]
                term_dict[index_type]["cfc_weight"] = word_list_cfc_weights[word_list_index]
            else:
                print "ERROR: word_list_words[word_list_index] == term_dict[\"word\"]"
                exit(0)


read_stopword_list(stopword_list_file_path)
read_queries(query_file_path, True)
del stopword_list_words
read_word_list(word_list_file_path, "MAIN")
get_reletad_word_statistics_from_word_list("MAIN")
del word_list_words
del word_list_occurance_in_docs
del word_list_cfc_weights
del word_list_disk_addresses
read_word_list(word_list_file_path_topic_based_clusters_csi, "TOPIC_CSI")
get_reletad_word_statistics_from_word_list("TOPIC_CSI")
del word_list_words
del word_list_occurance_in_docs
del word_list_cfc_weights
del word_list_disk_addresses

# QUERY FILE FORMAT
# <query_length>
# <occurange_in_query>
# <occurance_in_docs> <cfc_weight> <disk_addresses>
# ..
# <occurange_in_query>
# <occurance_in_docs> <cfc_weight> <disk_addresses>
# ..
# <query_length>
# <occurange_in_query>
# <occurance_in_docs> <cfc_weight> <disk_addresses>
# ..
# <occurange_in_query>
# <occurance_in_docs> <cfc_weight> <disk_addresses>

output_1 = open("queries_MAIN.txt","w")
output_2 = open("queries_TOPIC.txt","w")
for query_index in xrange(len(query_lengths)):
    output_1.write(str(query_lengths[query_index]) + "\n")
    output_2.write(str(query_lengths[query_index]) + "\n")
    for term_index in xrange(len(query_terms[query_index + 1])):
        output_1.write(str(query_terms[query_index + 1][term_index]["occurance_in_query"]) + "\n")
        output_1.write(str(query_terms[query_index + 1][term_index]["MAIN"]["occurance_in_docs"]) + " ")
        output_1.write(str(query_terms[query_index + 1][term_index]["MAIN"]["cfc_weight"]) + " ")
        output_1.write(str(query_terms[query_index + 1][term_index]["MAIN"]["disk_address"]) + "\n")

        output_2.write(str(query_terms[query_index + 1][term_index]["occurance_in_query"]) + "\n")
        output_2.write(str(query_terms[query_index + 1][term_index]["TOPIC_CSI"]["occurance_in_docs"]) + " ")
        output_2.write(str(query_terms[query_index + 1][term_index]["TOPIC_CSI"]["cfc_weight"]) + " ")
        output_2.write(str(query_terms[query_index + 1][term_index]["TOPIC_CSI"]["disk_address"]) + "\n")
output_1.close()
output_2.close()

# print query_lengths
# print query_terms

print "Script Ended at: " + time.strftime('%X %x')