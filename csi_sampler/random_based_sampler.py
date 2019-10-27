import os
import sys
import random


__SAMPLE_PERCENTAGE__ = 0.01
__NO_OF_CLUSTERS__ = 100
clusters_directory						= "/home1/grupef/TopicBasedClusters_100"
sampled_docs_file_create_path			= "/home1/grupef/TopicBasedClusters_100_sampled_doc_ids"

sampled_docs_file = open(sampled_docs_file_create_path,"w")

cluster_files = [ os.path.join(clusters_directory,"read",f) for f in os.listdir(os.path.join(clusters_directory,"read")) if os.path.isfile(os.path.join(os.path.join(clusters_directory,"read"),f)) ]


for cluster_file in cluster_files:
	cluster_file_desc = open(cluster_file)
	cluster_file_read = cluster_file_desc.readlines()
	cluster_file_desc.close()

	cluster_length = len(cluster_file_read)
	cluster_sample_length = cluster_length*__SAMPLE_PERCENTAGE__

	sampled_doc_ids = []
	i = 0
	while (i < cluster_sample_length):

		random_doc_index = random.randint(0,cluster_length-1)
		random_doc_id = cluster_file_read[random_doc_index]

		if not random_doc_id in sampled_doc_ids:
			sampled_doc_ids.append(random_doc_id)
			i = i + 1

	# No need for sort. After writing every sampled doc id to a file, file will be sort anyway.
	# sorted_doc_ids = sorted(sampled_doc_ids)

	for sampled_doc_id in sampled_doc_ids:
		sampled_docs_file.write(str(sampled_doc_id))
		

sampled_docs_file.close()