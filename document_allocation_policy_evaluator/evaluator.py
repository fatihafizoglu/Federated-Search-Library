import os
import sys
import re
import time

__PRECISIONS__ = [10,20,30]
__NUMBERS_OF_SELECTED_SHARDS__ = [1,3,5,10,15]
__TOP_K_DOCUMENT_FROM_CSI__ = 1000

clusters_directory						= "/media/arcelik/UbuntuExtra/TopicBasedClusters_100"
results_on_original_index				= "/home/arcelik/git/document-allocation-policies/document_allocation_policy_evaluator/trec-top1000.txt"
results_on_CSI							= "/home/arcelik/git/document-allocation-policies/document_allocation_policy_evaluator/trec-top1000.txt"

class DocClusterMap:

	def __init__(self,file_name):

		file_descriptor = open(file_name)
		file_read = file_descriptor.readlines()
		file_descriptor.close()

		self.doc_cluster_map = []

		for index in xrange(len(file_read)):
			line_splitted = file_read[index].split(" ")
			self.doc_cluster_map.append(int(line_splitted[1]))

	def get_cluster_id(self,document_id):

		return self.doc_cluster_map[document_id-1]

class QueryResults:
	
	def __init__(self,file_name):

		file_descriptor = open(file_name)
		file_read = file_descriptor.read()
		file_descriptor.close()

		file_parsed = re.findall("([0-9]*)?\t.*?\t([0-9]*)?\t[0-9]*\t([0-9]*.[0-9]*)?\t.*?",file_read)

		self.query_results = []

		latest_query_id = 0
		for parsed_line in file_parsed:

			query_id = int(parsed_line[0])
			if query_id != latest_query_id:
				while(latest_query_id < query_id):
					self.query_results.append([])
					latest_query_id += 1

			self.query_results[query_id-1].append((int(parsed_line[1]),float(parsed_line[2])))

	def get_results(self,query_id,result_size):
		
		return self.query_results[query_id-1][0:result_size]

	def get_query_count(self):

		return len(self.query_results)

class ResourceSelectionRedde:

	def __init__(self,query_results,doc_cluster_map):

		cluster_doc_count_dict = {}
		for (doc_id,doc_score) in query_results:
			cluster_id = doc_cluster_map.get_cluster_id(doc_id)
			try:
				cluster_doc_count_dict[cluster_id] += 1
			except KeyError:
				cluster_doc_count_dict[cluster_id] = 1

		self.sorted_clusters = sorted(cluster_doc_count_dict.items(),key = lambda x:x[1])
		self.sorted_clusters.reverse()

	def get_top_k_clusters(self,k):

		return [x[0] for x in self.sorted_clusters[0:k]]

class PrecisionEvaluator:

	def __init__(self,cluster_list,query_results):

		self.cluster_list = cluster_list
		self.query_results = query_results

	def get_precision(self):

		found = 0

		for (doc_id,doc_score) in self.query_results:
			cluster_id = doc_cluster_map.get_cluster_id(doc_id)
			if cluster_id in self.cluster_list:
				found +=1

		return float(found)/len(self.query_results)





doc_cluster_map = DocClusterMap(os.path.join(clusters_directory,"read","final","cluster_concat_sorted"))

query_results_on_original_index	= QueryResults(results_on_original_index)
query_results_on_CSI			= QueryResults(results_on_CSI)

if query_results_on_original_index.get_query_count() != query_results_on_CSI.get_query_count():
	print "Query Counts Does Not Match!"

for query_index in range(query_results_on_original_index.get_query_count()):
	
	top_k_results_on_CSI = query_results_on_CSI.get_results(query_index+1,__TOP_K_DOCUMENT_FROM_CSI__)

	resource_selection_redde = ResourceSelectionRedde(top_k_results_on_CSI,doc_cluster_map)

	for precision in __PRECISIONS__:
		for no_of_selected_shard in __NUMBERS_OF_SELECTED_SHARDS__:
			
			cluster_list = resource_selection_redde.get_top_k_clusters(no_of_selected_shard)
			query_results = query_results_on_original_index.get_results(query_index+1,precision)
			
			precision_evaluator = PrecisionEvaluator(cluster_list,query_results)
			precision_result = precision_evaluator.get_precision()

			print str(query_index+1) + " " + str(no_of_selected_shard) + " " + str(precision) + " " + str(precision_result)