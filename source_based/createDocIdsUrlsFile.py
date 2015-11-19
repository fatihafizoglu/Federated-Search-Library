import sys
import os
import re
import time

def print_usage():
	print "\nWRONG USAGE:"
	print "Usage: python sourceBasedClusterBuilder.py [IDP]"
	print "\t IDP           Input Directory Path"
	print "Ex:   python createDocIdsUrlsFile.py /media/arcelik/UbuntuExtra/govDatExtracted"

if len(sys.argv) < 2:
	print_usage()
	sys.exit()

print "Script started at: " + time.strftime('%X %x')

ARGV_INPUT_DIRECTORY = sys.argv[1]

# Open a file to append urls exctracted from documents
urls_file = open("urls.txt","w")

for directory_name in os.listdir(ARGV_INPUT_DIRECTORY):
	for file_name in os.listdir(os.path.join(ARGV_INPUT_DIRECTORY,directory_name)):
		print file_name
		if ".gz" in file_name: # For skipping zipped files
			continue
		
		gov_dat_partition = open(os.path.join(ARGV_INPUT_DIRECTORY,directory_name,file_name))
		gov_dat_partition_read = gov_dat_partition.read()
		gov_dat_partition.close()

		documents = re.findall("<DOC>.*?</DOC>[^\<]",gov_dat_partition_read,re.S)

		for document in documents:
			doc_id = re.findall("<DOCNO>(.*?)<\/DOCNO>",document)[0]
			doc_url = re.findall("<DOCHDR>\n([\w].*?)\n",document)[0]

			urls_file.write(doc_url + " " + doc_id + "\n")

urls_file.close()

# Sort urls.txt file with unix sort
os.system("sort urls.txt > sorted_urls.txt")

print "Script ended at:   " + time.strftime('%X %x')