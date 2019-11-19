import os
import sys
import struct
import time

## DONT USE THIS! check msworks proj

# #
# Globals
__MAX_MEMORY_READ__ = 10 # In GB
__MAX_MEMORY_WRITE__ = 5 # In GB
__INTEGER_SIZE__ = 4
sampled_docs_ids_file_path				= "/home1/grupef/ecank/data/CSI/nospam_access_topk_csi_docids"
#sampled_docs_ids_file_path				= "/home1/grupef/ecank/data/CSI/nospam_pr_topk_csi_docids"
#sampled_docs_ids_file_path				= "/home1/grupef/ecank/data/CSI/nospam_random_csi_docids"
sampled_docs_index_file_create_path		= "/home1/grupef/ecank/data/temp/inverted_index"
sampled_docs_wordlist_create_path		= "/home1/grupef/ecank/data/temp/wordlist"
index_file								= "/home1/sengor/CLUEWEB/createIIS/merged_entry.txt"
word_list_file							= "/home1/sengor/CLUEWEB/createIIS/merged_wordlist.txt"

print "Script Started at: " + time.strftime('%X %x')

# #
# This function is an iterator: Yields (term,posting_list_length) each time next called.
def next_word_posting_list_length():
	with open(word_list_file) as infile:
		for line in infile:
			line_splitted = line.split(" ")
			yield (line_splitted[0],int(line_splitted[1]))

# #
# This function is an iterator: Reads index file as chunks and yields (doc_id,occurance_count,8byte<doc_id-occurance_count>) each time next called.
def bytes_from_file(filename, chunksize=8192):
	with open(filename, "rb") as f:
		while True:
			chunk = f.read(chunksize)
			# print pympler.asizeof.asizeof(chunk)
			print "New chunk just read!: " + str(chunksize)
			if chunk:
				byte_string = ""
				for byte in chunk:
					byte_string += byte
					if len(byte_string) == __INTEGER_SIZE__ * 2:
						# yield (struct.unpack("I", byte_string[0:4])[0],struct.unpack("I", byte_string[4:8])[0])
						(doc_id,occurance_count) = struct.unpack("II", byte_string)
						yield (doc_id,occurance_count,byte_string)
						byte_string = ""
			else:
				break

# #
# This function returns an array: Sampled documents array. Sorted.
def read_sampled_document_ids():

	sampled_docs_ids_file = open(sampled_docs_ids_file_path)
	sampled_docs_ids_file_read = sampled_docs_ids_file.readlines()
	sampled_docs_ids_file.close()

	to_return = []
	for index in xrange(len(sampled_docs_ids_file_read)):
		to_return.append(int(sampled_docs_ids_file_read[index]))

	return to_return
# #
# Binary search function: This function search a document id in sampled documents array.
def binary_search_document_ids(item):
	first = 0
	last = len(sampled_docs_ids)-1
	found = False

	while first<=last and not found:
		midpoint = (first + last)//2
		if sampled_docs_ids[midpoint] == item:
			found = True
		else:
			if item < sampled_docs_ids[midpoint]:
				last = midpoint-1
			else:
				first = midpoint+1
	return found



sampled_docs_ids = read_sampled_document_ids()
next_word_posting_list_length_getter = next_word_posting_list_length()

sampled_docs_index_file = open(sampled_docs_index_file_create_path,"wb")
sampled_docs_wordlist_file = open(sampled_docs_wordlist_create_path,"w")

sampled_docs_binaries = []
sampled_docs_word_list = []
sampled_docs_word_occurance = 0


(latest_word,posting_list_length) = next_word_posting_list_length_getter.next()
total_size_of_posting_lists_in_memory = posting_list_length
for (doc_id,occurance_count,byte_string) in bytes_from_file(index_file,__MAX_MEMORY_READ__*1024*1024*1024): # 10GB per read!

	if posting_list_length == 0:
		# if sampled_docs_word_occurance > 0:
		# Non existing words will be on wordlist file with no of occurance as 0
		sampled_docs_word_list.append(latest_word + " " + str(sampled_docs_word_occurance) + " -1" + "\n")
		sampled_docs_word_occurance = 0

		if total_size_of_posting_lists_in_memory * 8 > __MAX_MEMORY_WRITE__*1024*1024*1024: # Larger than 5GB data in memory!
			print "Writing Started"
			for binary_string in sampled_docs_binaries:
				sampled_docs_index_file.write(binary_string)
			for word_list_string in sampled_docs_word_list:
				sampled_docs_wordlist_file.write(word_list_string)

			sampled_docs_binaries = []
			sampled_docs_word_list = []

			total_size_of_posting_lists_in_memory = 0
			print "Writing Ended"

		(latest_word,posting_list_length) = next_word_posting_list_length_getter.next()
		total_size_of_posting_lists_in_memory += posting_list_length


	posting_list_length = posting_list_length - 1

	if binary_search_document_ids(doc_id):
		sampled_docs_binaries.append(byte_string)
		sampled_docs_word_occurance += 1

# if sampled_docs_word_occurance > 0:
# Non existing words will be on wordlist file with no of occurance as 0
sampled_docs_word_list.append(latest_word + " " + str(sampled_docs_word_occurance) + " -1" + "\n")
print "Last Writing Started"
for binary_string in sampled_docs_binaries:
	sampled_docs_index_file.write(binary_string)
for word_list_string in sampled_docs_word_list:
	sampled_docs_wordlist_file.write(word_list_string)
sampled_docs_binaries = []
sampled_docs_word_list = []
print "Last Writing Ended"
sampled_docs_index_file.close()
sampled_docs_wordlist_file.close()

print "Script ended at:   " + time.strftime('%X %x')
