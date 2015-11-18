import sys
import os
import re
import random
import math
import json

def print_usage():
	print "\nWRONG USAGE:"
	print "Usage: python randomClusterBuilder.py [IDP] [ODP] [CS] [MS]"
	print "\nArguments"
	print "\t IDP           Input Directory Path"
	print "\t ODP           Output Directory Path"
	print "\t NOC           Number of Clusters"
	print "\t MS            Maximum number of document in one large file"
	print "Ex:   python randomClusterBuilder.py /media/arcelik/UbuntuExtra/govDatExtracted /media/arcelik/UbuntuExtra/RandomClusters 50 1000"

if len(sys.argv) < 5:
	print_usage()
	sys.exit()

ARGV_INPUT_DIRECTORY = sys.argv[1]
ARGV_OUTPUT_DIRECTORY = sys.argv[2]
ARGV_NO_OF_CLUSTER = int(sys.argv[3])
ARGV_MAX_NO_OF_DOCUMENT_IN_A_FILE = int(sys.argv[4])

if os.path.exists(ARGV_OUTPUT_DIRECTORY):
	print "cannot create output directory '" + ARGV_OUTPUT_DIRECTORY + "': Directory exists"
	sys.exit()
else:
	os.mkdir(ARGV_OUTPUT_DIRECTORY)
	for i in range(ARGV_NO_OF_CLUSTER):
		os.mkdir(os.path.join(ARGV_OUTPUT_DIRECTORY,str(i)))
	
cluster_document_dict = {}
cluster_size_dict = {}

for directory_name in os.listdir(ARGV_INPUT_DIRECTORY):
	for file_name in os.listdir(os.path.join(ARGV_INPUT_DIRECTORY,directory_name)):
		if ".gz" in file_name: # For skipping zipped files
			continue

		gov_dat_partition = open(os.path.join(ARGV_INPUT_DIRECTORY,directory_name,file_name))
		gov_dat_partition_read = gov_dat_partition.read()
		gov_dat_partition.close()

		documents = re.findall("<DOC>.*?</DOC>[^\<]",gov_dat_partition_read,re.S)

		for document in documents:
			
			random_cluster_id = random.randint(0,ARGV_NO_OF_CLUSTER - 1)
			output_file_name = 0

			if cluster_size_dict.has_key(random_cluster_id):
				output_file_name = cluster_size_dict[random_cluster_id] / ARGV_MAX_NO_OF_DOCUMENT_IN_A_FILE
				cluster_size_dict[random_cluster_id] = cluster_size_dict[random_cluster_id] + 1
			else:
				cluster_size_dict[random_cluster_id] = 1

			doc_id = re.findall("<DOCNO>(.*?)<\/DOCNO>",document)[0]
			cluster_document_dict[doc_id] = random_cluster_id

			cluster_documents_file = open(os.path.join(ARGV_OUTPUT_DIRECTORY,str(random_cluster_id),str(output_file_name)),"a")
			cluster_documents_file.write(document)
			cluster_documents_file.close()
			# TODO: Dosyalari acip acip kapatmaya gerek yok. Max no of file a erisene kadar acik kalabilir. Performance Improvement if needed

with open(os.path.join(ARGV_OUTPUT_DIRECTORY,'document_id_cluster_id_map.json'), 'w') as fp:
	json.dump(cluster_document_dict, fp)