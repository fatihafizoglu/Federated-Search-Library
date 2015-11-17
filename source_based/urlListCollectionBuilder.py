import sys
import os
import re
import math

def print_usage():
	print "\nWRONG USAGE:"
	print "Usage: python urlListCollectionBuilder.py [IDP] [ODP] [METHOD] [CS|NOC]"
	print "\nArguments"
	print "\t IDP           Input Directory Path"
	print "\t ODP           Output Directory Path"
	print "\t METHOD        Set either FixedCollectionSize or FixedNumberOfCollection"
	print "\t CS|NOC        If METHOD is FixedCollectionSize,CS-Collection Size"
	print "\t               If METHOD is FixedNumberOfCollection,NOC-Number of Collection"
	print "Ex:   python urlListCollectionBuilder.py /media/arcelik/UbuntuExtra/govDatExtracted /media/arcelik/UbuntuExtra/govDatNewCollections FixedCollectionSize 1000"
	# Suggested usage from paper below
	print "Ex:   python urlListCollectionBuilder.py /media/arcelik/UbuntuExtra/govDatExtracted /media/arcelik/UbuntuExtra/govDatNewCollections FixedNumberOfCollection 100"

def create_urls_files():

	# Open a file to append urls exctracted from documents
	urls_file = open("urls.txt","w")

	# Traverse original govDat documents, extract urls from documents and append them to urls.txt
	for filename in os.listdir(ARGV_INPUT_DIRECTORY):
		gov_dat_partition = open(ARGV_INPUT_DIRECTORY + "/" + filename)
		gov_dat_partition_read = gov_dat_partition.read()
		gov_dat_partition.close()

		documents = re.findall("<DOC>.*?</DOC>[^\<]",gov_dat_partition_read,re.S)

		for document in documents:
			
			doc_id = re.findall("<DOCNO>(.*?)<\/DOCNO>",document)[0]
			doc_url = re.findall("<DOCHDR>\n.*?([\w]*?\.gov.*)",document)[0]

			urls_file.write(doc_url + " " + doc_id + "\n")

	urls_file.close()

	# Sort urls.txt file with unix sort
	os.system("sort urls.txt > sorted_urls.txt")

def create_collections():

	# Open sorted_urls.txt which is one of the outputs of phase1 - create_urls_files
	sorted_urls = open("sorted_urls.txt")
	sorted_urls_read = sorted_urls.read().split("\n")
	sorted_urls.close()

	# Total number of documents, number of collections and collection sizes needed for 
	# further computation. Obtaining these information can change with the given method
	TOTAL_DOCUMENT = len(sorted_urls_read)
	if ARGV_METHOD == "FixedCollectionSize":
		
		ARGV_COLLECTION_SIZE = int(sys.argv[4])
		
		COLLECTION_SIZE = ARGV_COLLECTION_SIZE
		NO_OF_COLLECTION = int(math.ceil(float(TOTAL_DOCUMENT) / COLLECTION_SIZE))

	elif ARGV_METHOD == "FixedNumberOfCollection":
		
		ARGV_NO_OF_COLLECTION = int(sys.argv[4])
		
		COLLECTION_SIZE = int(math.ceil(float(TOTAL_DOCUMENT) / ARGV_NO_OF_COLLECTION))
		NO_OF_COLLECTION = ARGV_NO_OF_COLLECTION

	# docid_collection_dict is a map for creating doc_id -> collection_id pairs
	docid_collection_dict = {}
	for index,url_docid in enumerate(sorted_urls_read):
		if url_docid == '':
			continue
		docid = url_docid.split(" ")[1]
		docid_collection_dict[docid] = index / COLLECTION_SIZE

	# Create directories for new collections
	os.mkdir(ARGV_OUTPUT_DIRECTORY)
	for i in range(NO_OF_COLLECTION):
		os.mkdir(ARGV_OUTPUT_DIRECTORY + "/" + str(i))	

	# Traverse original govDat documents, extract doc_ids from documents and find their
	# mapped collections from docid_collection_dict and copy to collection folder.
	for filename in os.listdir(ARGV_INPUT_DIRECTORY):
		gov_dat_partition = open(ARGV_INPUT_DIRECTORY + "/" + filename)
		gov_dat_partition_read = gov_dat_partition.read()
		gov_dat_partition.close()

		documents = re.findall("<DOC>.*?</DOC>[^\<]",gov_dat_partition_read,re.S)

		for document in documents:
			
			doc_id = re.findall("<DOCNO>(.*?)<\/DOCNO>",document)[0]
			collection_id = docid_collection_dict[doc_id]

			document_file = open(ARGV_OUTPUT_DIRECTORY + "/" + str(collection_id) + "/1","a")
			document_file.write(document)
			document_file.close()


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


decision = raw_input("""First pass of the program is about to start. This pass will produce sorted list of urls of documents and their doc_ids. Result of this pass will be stored in sorted_urls.txt. Do you want to continue? (Y/n): """)
if decision == "Y":
	create_urls_files()
else:
	print """Program assumes urls.txt already created and will continue from there."""

decision = raw_input("""Second pass of the program is about to start. This pass will create clusters with respect to given options. Do you want to continue? (Y/n): """)
if decision == "Y":
	create_collections()
else:
	print """Second pass of the program not executed."""