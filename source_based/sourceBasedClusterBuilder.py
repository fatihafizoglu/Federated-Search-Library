import sys
import os
import re
import math
import json
import time

def print_usage():
	print "\nWRONG USAGE:"
	print "Usage: python sourceBasedClusterBuilder.py [IDP] [ODP] [NOC] [MS] [NOD]"
	print "\nArguments"
	print "\t IDP           Input Directory Path"
	print "\t ODP           Output Directory Path"
	print "\t NOC           Number of Clusters"
	print "\t MS            Maximum number of document in one large file"
	print "\t NOD           Number of documents in collection"
	print "Ex:   python sourceBasedClusterBuilder.py /media/arcelik/UbuntuExtra/govDatExtracted /media/arcelik/UbuntuExtra/SourceBasedClusters 50 1000 89771"

if len(sys.argv) < 6:
	print_usage()
	sys.exit()

print "Script started at: " + time.strftime('%X %x')

ARGV_INPUT_DIRECTORY = sys.argv[1]
ARGV_OUTPUT_DIRECTORY = sys.argv[2]
ARGV_NO_OF_CLUSTER = int(sys.argv[3])
ARGV_MAX_NO_OF_DOCUMENT_IN_A_FILE = int(sys.argv[4])
ARGV_NO_OF_DOCUMENTS_IN_COLLECTION = int(sys.argv[5])
CLUSTER_SIZE = int(math.ceil(float(ARGV_NO_OF_DOCUMENTS_IN_COLLECTION) / ARGV_NO_OF_CLUSTER))

if os.path.exists(ARGV_OUTPUT_DIRECTORY):
	print "cannot create output directory '" + ARGV_OUTPUT_DIRECTORY + "': Directory exists"
	sys.exit()
else:
	os.mkdir(ARGV_OUTPUT_DIRECTORY)
	for i in range(ARGV_NO_OF_CLUSTER):
		os.mkdir(os.path.join(ARGV_OUTPUT_DIRECTORY,str(i)))

cluster_document_dict = {}
cluster_size_dict = {}
index = 0
# Open sorted_urls.txt which is one of the outputs of phase1 - create_urls_files
with open("sorted_urls.txt") as f:
	for url_docid in f:
		if url_docid == "":
			continue
		doc_id = url_docid.split("\n")[0].split(" ")[1]
		cluster_document_dict[doc_id] = index / CLUSTER_SIZE
		index = index + 1

for directory_name in os.listdir(ARGV_INPUT_DIRECTORY):
	for file_name in os.listdir(os.path.join(ARGV_INPUT_DIRECTORY,directory_name)):
		if ".gz" in file_name: # For skipping zipped files
			continue

		gov_dat_partition = open(os.path.join(ARGV_INPUT_DIRECTORY,directory_name,file_name))
		gov_dat_partition_read = gov_dat_partition.read()
		gov_dat_partition.close()

		documents = re.findall("<DOC>.*?</DOC>[^\<]",gov_dat_partition_read,re.S)

		for document in documents:

			doc_id = re.findall("<DOCNO>(.*?)<\/DOCNO>",document)[0]
			cluster_id = cluster_document_dict[doc_id]
			output_file_name = 0

			if cluster_size_dict.has_key(cluster_id):
				output_file_name = cluster_size_dict[cluster_id] / ARGV_MAX_NO_OF_DOCUMENT_IN_A_FILE
				cluster_size_dict[cluster_id] = cluster_size_dict[cluster_id] + 1
			else:
				cluster_size_dict[cluster_id] = 1

			cluster_documents_file = open(os.path.join(ARGV_OUTPUT_DIRECTORY,str(cluster_id),str(output_file_name)),"a")
			cluster_documents_file.write(document)
			cluster_documents_file.close()
			# TODO: Dosyalari acip acip kapatmaya gerek yok. Max no of file a erisene kadar acik kalabilir. Performance Improvement if needed

with open(os.path.join(ARGV_OUTPUT_DIRECTORY,'document_id_cluster_id_map.json'), 'w') as fp:
	json.dump(cluster_document_dict, fp)

print "Script ended at:   " + time.strftime('%X %x')