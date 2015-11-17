import sys
import os
import re
import random
import math

def print_usage():
	print "\nWRONG USAGE:"
	print "Usage: python randomCollectionBuilder.py [IDP] [ODP] [METHOD] [CS|NOC] [NOD]"
	print "\nArguments"
	print "\t IDP           Input Directory Path"
	print "\t ODP           Output Directory Path"
	print "\t METHOD        Set either FixedCollectionSize or FixedNumberOfCollection"
	print "\t CS|NOC        If METHOD is FixedCollectionSize,CS-Collection Size"
	print "\t               If METHOD is FixedNumberOfCollection,NOC-Number of Collection"
	print '\t NOD			If METHOD is FixedCollectionSize,NOD-Number of Documents in Total'
	print "Ex:   python randomCollectionBuilder.py /media/arcelik/UbuntuExtra/govDatExtracted /media/arcelik/UbuntuExtra/govDatNewCollections FixedCollectionSize 1000 89771"
	print "Ex:   python randomCollectionBuilder.py /media/arcelik/UbuntuExtra/govDatExtracted /media/arcelik/UbuntuExtra/govDatNewCollections FixedNumberOfCollection 100"

if len(sys.argv) < 5:
	print_usage()
	sys.exit()

ARGV_INPUT_DIRECTORY = sys.argv[1]
ARGV_OUTPUT_DIRECTORY = sys.argv[2]
if os.path.exists(ARGV_OUTPUT_DIRECTORY):
		print "cannot create output directory '" + ARGV_OUTPUT_DIRECTORY + "': Directory exists"
		sys.exit()
ARGV_METHOD = sys.argv[3]
if ARGV_METHOD != "FixedCollectionSize" and ARGV_METHOD != "FixedNumberOfCollection":
	print_usage()
	sys.exit()

collection_size_dict = {}

if ARGV_METHOD == "FixedCollectionSize":
	
	if len(sys.argv) < 6:
		print_usage()
		sys.exit()

	ARGV_COLLECTION_SIZE = int(sys.argv[4])
	ARGV_TOTAL_DOCUMENT = int(sys.argv[5])

	NO_OF_COLLECTION = int(math.ceil(float(ARGV_TOTAL_DOCUMENT) / ARGV_COLLECTION_SIZE))
	for i in range(NO_OF_COLLECTION):
		collection_size_dict[i] = 0
	
	os.mkdir(ARGV_OUTPUT_DIRECTORY)
	for i in range(NO_OF_COLLECTION):
		os.mkdir(ARGV_OUTPUT_DIRECTORY + "/" + str(i))

	for filename in os.listdir(ARGV_INPUT_DIRECTORY):
		gov_dat_partition = open(ARGV_INPUT_DIRECTORY + "/" + filename)
		gov_dat_partition_read = gov_dat_partition.read()
		gov_dat_partition.close()

		documents = re.findall("<DOC>.*?</DOC>[^\<]",gov_dat_partition_read,re.S)

		for document in documents:
			
			random_directory_id = random.choice(collection_size_dict.keys())
			collection_size_dict[random_directory_id] = collection_size_dict[random_directory_id] + 1
			if collection_size_dict[random_directory_id] == ARGV_COLLECTION_SIZE:
				del collection_size_dict[random_directory_id]

			document_file = open(ARGV_OUTPUT_DIRECTORY + "/" + str(random_directory_id) + "/1","a")
			document_file.write(document)
			document_file.close()


elif ARGV_METHOD == "FixedNumberOfCollection":
	
	ARGV_NO_OF_COLLECTION = int(sys.argv[4])
	
	os.mkdir(ARGV_OUTPUT_DIRECTORY)
	for i in range(ARGV_NO_OF_COLLECTION):
		os.mkdir(ARGV_OUTPUT_DIRECTORY + "/" + str(i))

	for filename in os.listdir(ARGV_INPUT_DIRECTORY):
		gov_dat_partition = open(ARGV_INPUT_DIRECTORY + "/" + filename)
		gov_dat_partition_read = gov_dat_partition.read()
		gov_dat_partition.close()

		documents = re.findall("<DOC>.*?</DOC>[^\<]",gov_dat_partition_read,re.S)

		for document in documents:
			
			random_directory_id = random.randint(0,ARGV_NO_OF_COLLECTION - 1)
			
			document_file = open(ARGV_OUTPUT_DIRECTORY + "/" + str(random_directory_id) + "/1","a")
			document_file.write(document)
			document_file.close()