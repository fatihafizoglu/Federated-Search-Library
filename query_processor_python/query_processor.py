import numpy as np
import os.path
import struct
import heapq
import math
import time

from resourceselectionlib import Redde

print "Script Started at: " + time.strftime('%X %x')

DUMP_LOCATION = "/home/fatihafizoglu/git/MS/document-allocation-policies/query_processor_python/saved/"
DOC_ID_TERM_FREQUENCY_PAIR_SIZE_IN_BYTES = 8
WORD_NO = 163629159
DOC_NO = 50220539
STOPWORD_NO = 423
BEST_DOCS = 1000
BEST_DOCS_CSI = 100
NO_OF_RESOURCE = 10
NO_OF_CLUSTERS = 100

ELEMINATE_SPAMS = True
SPAM_THRESHOLD = 60

METHOD_CONSTANTS = {
                        "BM25" : {"k1" : 1.2, "b" : 0.5, "k1+1" : 2.2, "1-b" : 0.5}
                    }

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

doc_length_file_path = "/media/fatihafizoglu/LenovoMS/MS/Index/merged_SMART-documents.txt"
doc_length_unique_term_occurances = None
doc_length_term_occurances = None
doc_length_statistics = None

document_spam_scores_file_path = "/media/fatihafizoglu/LenovoMS/MS/spamsMap.bin"
document_spam_scores = None

document_cluster_map_topic_file_path = "/media/fatihafizoglu/LenovoMS/MS/TopicBasedClusters_100_2/read/final/cluster_concat_sorted"
document_cluster_map_random_file_path = "/media/fatihafizoglu/LenovoMS/MS/RandomBasedClusters_100/read/final/cluster_concat_sorted"
document_cluster_maps = {}

index_main_file_path = "/media/fatihafizoglu/LenovoMS/MS/Index/merged_entry.txt"
index_topic_csi_file_path = "/media/fatihafizoglu/LenovoMS/MS/Index/merged_entry_TopicBasedClusters_100_2_CSI.txt"
selected_resources = {}

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

def read_doc_lengths(path):
    global doc_length_unique_term_occurances
    global doc_length_term_occurances
    global doc_length_statistics

    doc_length_unique_term_occurances = np.empty(DOC_NO,dtype=np.uint32)
    doc_length_term_occurances = np.empty(DOC_NO,dtype=np.uint32)
    doc_length_statistics = {"doc_length_total_term_occurance" : 0, "doc_length_total_unique_term_occurance" : 0, "doc_length_average_total_term_occurance" : 0.0, "doc_length_average_unique_term_occurance" : 0.0, "doc_length_total_doc_number_with_atleast_more_than_one_unique_term" : 0}

    doc_length_unique_term_occurances_dump_path = os.path.join(DUMP_LOCATION, "doc_length_unique_term_occurances.npy")
    doc_length_term_occurances_dump_path = os.path.join(DUMP_LOCATION, "doc_length_term_occurances.npy")
    doc_length_statistics_dump_path = os.path.join(DUMP_LOCATION, "doc_length_statistics.npy")

    if os.path.exists(doc_length_unique_term_occurances_dump_path) and os.path.exists(doc_length_term_occurances_dump_path) and os.path.exists(doc_length_statistics_dump_path):
        print "INFO: Loading " + path
        doc_length_unique_term_occurances = np.load(doc_length_unique_term_occurances_dump_path)
        doc_length_term_occurances = np.load(doc_length_term_occurances_dump_path)
        doc_length_statistics = np.load(doc_length_statistics_dump_path).item()
        print "INFO: Finished " + path
    else:
        print "INFO: Reading " + path
        doc_length_file = open(path,'r')

        line_index = 1
        for line in doc_length_file:
            line_splitted = line.split()
            doc_id = int(line_splitted[0])
            doc_length_unique_term_occurances[line_index] = int(line_splitted[1])
            doc_length_term_occurances[line_index] = int(line_splitted[2])
            doc_length_statistics["doc_length_total_term_occurance"] += int(line_splitted[2])
            doc_length_statistics["doc_length_total_unique_term_occurance"] += int(line_splitted[1])
            if doc_id != line_index:
                print "ERROR: doc_id != line_index"
                exit(1)
            if doc_length_unique_term_occurances[line_index] > 0:
                doc_length_statistics["doc_length_total_doc_number_with_atleast_more_than_one_unique_term"] += 1
            line_index += 1
        doc_length_statistics["doc_length_average_total_term_occurance"] = float(doc_length_statistics["doc_length_total_term_occurance"]) / doc_length_statistics["doc_length_total_doc_number_with_atleast_more_than_one_unique_term"] # WHY NOT TOTAL DOC NUM?
        doc_length_statistics["doc_length_average_unique_term_occurance"] = float(doc_length_statistics["doc_length_total_unique_term_occurance"]) / doc_length_statistics["doc_length_total_doc_number_with_atleast_more_than_one_unique_term"]

        doc_length_file.close()
        print "INFO: Finished " + path

        np.save(doc_length_unique_term_occurances_dump_path, doc_length_unique_term_occurances)
        np.save(doc_length_term_occurances_dump_path, doc_length_term_occurances)
        np.save(doc_length_statistics_dump_path, doc_length_statistics)

def read_document_spam_scores(path):
    global document_spam_scores

    with open(path, 'rb') as document_spam_scores_file:
        document_spam_scores = np.fromfile(document_spam_scores_file, np.int8)

def read_document_cluster_map(path, clustering_strategy):
    global document_cluster_maps

    document_cluster_map = np.zeros(DOC_NO, dtype="uint8")

    document_cluster_map_dump_path = os.path.join(DUMP_LOCATION, clustering_strategy + "_document_cluster_map.npy")

    if (os.path.exists(document_cluster_map_dump_path)):
        document_cluster_map = np.load(document_cluster_map_dump_path)
    else:
        document_cluster_map_file = open(path, "r")
        line_index = 1
        for line in document_cluster_map_file:
            line_splitted = line.split()
            document_cluster_map[line_index] = int(line_splitted[1])
            line_index += 1

        np.save(document_cluster_map_dump_path, document_cluster_map)

    document_cluster_maps[clustering_strategy] = document_cluster_map

def score_docs_for_query(index_type, index_file, query_index):
    global doc_length_unique_term_occurances
    global doc_length_term_occurances
    global doc_length_statistics
    global query_terms
    global query_lengths

    result = {}
    for term_index_in_query, word_dict in enumerate(query_terms[query_index]):
        index_file.seek(word_dict[index_type]["disk_address"] * DOC_ID_TERM_FREQUENCY_PAIR_SIZE_IN_BYTES)

        term_occurance_in_docs = word_dict[index_type]["occurance_in_docs"]
        term_weight = math.log((doc_length_statistics["doc_length_total_doc_number_with_atleast_more_than_one_unique_term"] - term_occurance_in_docs + 0.5) / (term_occurance_in_docs + 0.5))
        doc_id_and_term_frequency_array_in_bytes = index_file.read(term_occurance_in_docs * DOC_ID_TERM_FREQUENCY_PAIR_SIZE_IN_BYTES)
        doc_id_and_term_frequency_array = struct.unpack('<' + str(term_occurance_in_docs * 2) + 'i', doc_id_and_term_frequency_array_in_bytes)
        for i in xrange(0, len(doc_id_and_term_frequency_array), 2):
            doc_id,term_frequency = doc_id_and_term_frequency_array[i:i+2]

            increment_score_amount = term_weight * ((term_frequency * METHOD_CONSTANTS["BM25"]["k1+1"]) / (term_frequency + METHOD_CONSTANTS["BM25"]["k1"] * (METHOD_CONSTANTS["BM25"]["1-b"] + (METHOD_CONSTANTS["BM25"]["b"] * (doc_length_term_occurances[doc_id] / doc_length_statistics["doc_length_average_total_term_occurance"])))))
            if term_index_in_query:
                try:
                    result[doc_id] += increment_score_amount
                except KeyError:
                    result[doc_id] = increment_score_amount
            else:
                result[doc_id] = increment_score_amount

    return result

def stats_docs_for_query(index_type, index_file, query_index):
    global query_terms

    result = {}
    for term_index_in_query, word_dict in enumerate(query_terms[query_index]):
        index_file.seek(word_dict[index_type]["disk_address"] * DOC_ID_TERM_FREQUENCY_PAIR_SIZE_IN_BYTES)

        term_occurance_in_docs = word_dict[index_type]["occurance_in_docs"]
        doc_id_and_term_frequency_array_in_bytes = index_file.read(term_occurance_in_docs * DOC_ID_TERM_FREQUENCY_PAIR_SIZE_IN_BYTES)
        doc_id_and_term_frequency_array = struct.unpack('<' + str(term_occurance_in_docs * 2) + 'i', doc_id_and_term_frequency_array_in_bytes)
        for i in xrange(0, len(doc_id_and_term_frequency_array), 2):
            doc_id = doc_id_and_term_frequency_array[i]

            if term_index_in_query:
                try:
                    result[doc_id] += 1
                except KeyError:
                    result[doc_id] = 1
            else:
                result[doc_id] = 1

    return result

def get_best_n(result, no_of_result):
    global document_spam_scores
    global document_cluster_maps

    heap = np.zeros(no_of_result, dtype='float, int32').tolist()
    heapq.heapify(heap)
    for document_index, score in result.iteritems():
        if ELEMINATE_SPAMS:
            if document_spam_scores[document_index] < SPAM_THRESHOLD:
                continue
        if heap[0][0] < score:
            heapq.heapreplace(heap, (score,document_index))

    heap = sorted(heap, key=lambda tup: tup[0])
    heap = [tup for tup in heap if tup[1] != 0]
    heap.reverse()

    return heap

def get_best_ns(result, no_of_result, selected_resource_list, strategies):
    global document_spam_scores
    global document_cluster_maps

    heaps = []
    for i in range(len(strategies)):
        heap = np.zeros(no_of_result, dtype='float, int32').tolist()
        heapq.heapify(heap)
        heaps.append(heap)

    for document_index, score in result.iteritems():
        for stragegy_index, stragegy in enumerate(strategies):
            if stragegy == "TOPIC" or stragegy == "RANDOM_AND_TOPIC":
                if document_cluster_maps["TOPIC"][document_index] not in selected_resource_list["TOPIC"]:
                    continue

            if ELEMINATE_SPAMS:
                if document_spam_scores[document_index] < SPAM_THRESHOLD:
                    continue
            
            if heaps[stragegy_index][0][0] < score:
                heapq.heapreplace(heaps[stragegy_index], (score,document_index))

    for i in range(len(strategies)):
        heaps[i] = sorted(heaps[i], key=lambda tup: tup[0])
        heaps[i] = [tup for tup in heaps[i] if tup[1] != 0]
        heaps[i].reverse()

    return heaps

def get_stats(result, selected_resource_list):
    global document_cluster_maps

    stats = [np.zeros(NO_OF_CLUSTERS, dtype='uint32'), np.zeros(NO_OF_CLUSTERS, dtype='uint32'), np.zeros(NO_OF_CLUSTERS, dtype='uint32'), np.zeros(NO_OF_CLUSTERS, dtype='uint32')]

    for document_index, value in result.iteritems():
        # EXHAUSTIVE
        stats[0][0] += value
        # TOPIC
        topic_cluster = document_cluster_maps["TOPIC"][document_index]
        random_cluster = document_cluster_maps["RANDOM"][document_index]
        
        if topic_cluster in selected_resource_list["TOPIC"]:
            stats[1][topic_cluster] += value
            stats[2][random_cluster] += value

        stats[3][random_cluster] += value

    print "EXHAUSTIVE       max :" + str(stats[0][0])
    print "EXHAUSTIVE       sum :" + str(stats[0][0])
    print "TOPIC            max :" + str(max([x for x in stats[1]]))
    print "TOPIC            sum :" + str(sum([x for x in stats[1]]))
    print "RANDOM_AND_TOPIC max :" + str(max([x for x in stats[2]]))
    print "RANDOM_AND_TOPIC sum :" + str(sum([x for x in stats[2]]))
    print "RANDOM           max :" + str(max([x for x in stats[3]]))
    print "RANDOM           sum :" + str(sum([x for x in stats[3]]))

def write_to_result_file(heap, result_file):
    for index, (score, doc_id) in enumerate(heap):
        if score > 0:
            result_file.write(str(query_index) + "\tQ0\t" + str(doc_id) + "\t" + str(index + 1) + "\t" + "{0:.6f}".format(score) + "\t" + "fs\n")
        else:
            break


# EXECUTE QUERY ON MAIN INDEX, GET SELECTABLE CLUSTERS FROM TOPIC_CSI, GET RESULTS FOR EXHAUSTIVE, TOPIC, RANDOM AND TOPIC, RANDOM
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
read_word_list(word_list_file_path_random_based_clusters_csi, "RANDOM_CSI")
get_reletad_word_statistics_from_word_list("RANDOM_CSI")
del word_list_words
del word_list_occurance_in_docs
del word_list_cfc_weights
del word_list_disk_addresses
# QUERIES READY

read_doc_lengths(doc_length_file_path)
read_document_spam_scores(document_spam_scores_file_path)
read_document_cluster_map(document_cluster_map_topic_file_path, "TOPIC")
read_document_cluster_map(document_cluster_map_random_file_path, "RANDOM")

index_topic_csi_file = open(index_topic_csi_file_path, "rb")
selected_resources["TOPIC"] = []
selected_resources["TOPIC"].append([])
for query_index in query_terms.keys():
    result = score_docs_for_query("TOPIC_CSI", index_topic_csi_file, query_index)
    heap = get_best_n(result, BEST_DOCS_CSI)
    resource_selection_method = Redde(heap, document_cluster_maps["TOPIC"])
    selected_resources["TOPIC"].append(resource_selection_method.get_top_k_clusters(NO_OF_RESOURCE))
index_topic_csi_file.close()

# # Get Results
# output_1 = open("result_EXHAUSTIVE.txt","w")
# output_2 = open("result_TOPIC.txt","w")
# output_3 = open("result_RANDOM_AND_TOPIC.txt","w")
# output_4 = open("result_RANDOM.txt","w")
# index_main_file = open(index_main_file_path, "rb")
# for query_index in query_terms.keys():
#     print query_index
#     result = score_docs_for_query("MAIN", index_main_file, query_index)
#     heaps = get_best_n(result, BEST_DOCS)
#     heaps = get_best_ns(result, BEST_DOCS, {"TOPIC" : selected_resources["TOPIC"][query_index]}, ["EXHAUSTIVE", "TOPIC", "RANDOM_AND_TOPIC", "RANDOM"])
#     write_to_result_file(heaps[0], output_1)
#     write_to_result_file(heaps[1], output_2)
#     write_to_result_file(heaps[2], output_3)
#     write_to_result_file(heaps[3], output_4)
# index_main_file.close()
# output_4.close()
# output_3.close()
# output_2.close()
# output_1.close()

# Get stats
index_main_file = open(index_main_file_path, "rb")
for query_index in query_terms.keys():
    print query_index
    result = stats_docs_for_query("MAIN", index_main_file, query_index)
    get_stats(result,{"TOPIC" : selected_resources["TOPIC"][query_index]})
index_main_file.close()


# # EXECUTE QUERY ON MAIN INDEX ONLY
# read_stopword_list(stopword_list_file_path)
# read_queries(query_file_path, True)
# del stopword_list_words
# read_word_list(word_list_file_path, "MAIN")
# get_reletad_word_statistics_from_word_list("MAIN")
# del word_list_words
# del word_list_occurance_in_docs
# del word_list_cfc_weights
# del word_list_disk_addresses
# # QUERIES READY

# read_doc_lengths(doc_length_file_path)
# read_document_spam_scores(document_spam_scores_file_path)

# output = open("result_EXHAUSTIVE.txt","w")
# index_main_file = open(index_main_file_path, "rb")
# for query_index in query_terms.keys():
#     print query_index
#     result = score_docs_for_query("MAIN", index_main_file, query_index)
#     heap = get_best_n(result, BEST_DOCS)
#     write_to_result_file(heap, output)
# index_main_file.close()
# output.close()



print "Script Ended at: " + time.strftime('%X %x')
