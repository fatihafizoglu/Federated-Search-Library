import numpy as np
import os
import struct
import heapq
import math
import time

from resourceselectionlib import Redde

print "Script Started at: " + time.strftime('%X %x')

DUMP_LOCATION = "/home/fatihafizoglu/git/MS/document-allocation-policies/query_processor_python/saved/"
DOC_ID_TERM_FREQUENCY_PAIR_SIZE_IN_BYTES = 8
DOC_NO = 50220539
BEST_DOCS_CSI = 100
NO_OF_RESOURCE = 10
NO_OF_CLUSTERS = 100
CLUSTERING_STRATEGY = "TOPIC"

ELEMINATE_SPAMS = True
SPAM_THRESHOLD = 60

METHOD_CONSTANTS = {
                        "BM25" : {"k1" : 1.2, "b" : 0.5, "k1+1" : 2.2, "1-b" : 0.5}
                    }


query_file_path = "queries_TOPIC_CSI.txt"
query_lengths = []
query_terms = {}

doc_length_file_path = "/media/fatihafizoglu/LenovoMS/MS/Index/merged_SMART-documents.txt"
doc_length_unique_term_occurances = None
doc_length_term_occurances = None
doc_length_statistics = None

document_spam_scores_file_path = "/media/fatihafizoglu/LenovoMS/MS/spamsMap.bin"
document_spam_scores = None

document_cluster_map_file_path = "/media/fatihafizoglu/LenovoMS/MS/TopicBasedClusters_100_2/read/final/cluster_concat_sorted"
document_cluster_map = None

csi_index_file_path = "/media/fatihafizoglu/LenovoMS/MS/Index/merged_entry_TopicBasedClusters_100_2_CSI.txt"
selected_resources = []

def read_queries(path):
    global query_lengths
    global query_terms

    query_file = open(path, 'r')

    query_index = 1
    while True:
        line = query_file.readline()

        if line:
            query_length = int(line)

            query_lengths.append(query_length)    
            query_terms[query_index] = []
            

            for i in range(query_length):
                term_hash = {}

                line = query_file.readline()
                occurance_in_query = int(line)

                term_hash["occurance_in_query"] = occurance_in_query

                line = query_file.readline()
                line_splitted = line.split()
                occurance_in_docs = int(line_splitted[0])
                cfc_weight = float(line_splitted[1])
                disk_address = int(line_splitted[2])

                term_hash["occurance_in_docs"] = occurance_in_docs
                term_hash["cfc_weight"] = cfc_weight
                term_hash["disk_address"] = disk_address

                query_terms[query_index].append(term_hash)
        else:
            break
        query_index += 1

    query_file.close()

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

def read_document_cluster_map(path):
    global document_cluster_map

    document_cluster_map = np.zeros(DOC_NO, dtype="uint8")

    document_cluster_map_dump_path = os.path.join(DUMP_LOCATION, CLUSTERING_STRATEGY + "_document_cluster_map.npy")

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

def score_docs_for_query(index_file, query_index):
    global doc_length_unique_term_occurances
    global doc_length_term_occurances
    global doc_length_statistics
    global query_terms
    global query_lengths

    result = {}
    for term_index_in_query, word_dict in enumerate(query_terms[query_index]):
        index_file.seek(word_dict["disk_address"] * DOC_ID_TERM_FREQUENCY_PAIR_SIZE_IN_BYTES)

        term_occurance_in_docs = word_dict["occurance_in_docs"]
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

def get_best_n(result, no_of_result):
    global document_spam_scores

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

read_queries(query_file_path)
read_doc_lengths(doc_length_file_path)
read_document_spam_scores(document_spam_scores_file_path)
read_document_cluster_map(document_cluster_map_file_path)

csi_index_file = open(csi_index_file_path, "rb")
selected_resources.append([])
for query_index in query_terms.keys():
    result = score_docs_for_query(csi_index_file, query_index)
    heap = get_best_n(result, BEST_DOCS_CSI)
    resource_selection_method = Redde(heap, document_cluster_map)
    selected_resources.append(resource_selection_method.get_top_k_clusters(NO_OF_RESOURCE))
csi_index_file.close()

output = open("selected_resources_TOPIC_CSI.txt","w")
for selected_resource in selected_resources[1:]:
    output.write(" ".join(str(resource_id) for resource_id in selected_resource))
    output.write("\n")
output.close()


print "Script Ended at: " + time.strftime('%X %x')