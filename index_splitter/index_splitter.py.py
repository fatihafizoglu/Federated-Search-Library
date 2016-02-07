import os
import sys
import struct
import time

# #
# Globals
__INTEGER_SIZE__ = 4
__NO_OF_CLUSTERS__ = 100
clusters_directory						= "/home1/grupef/TopicBasedClusters_100_2"
clusters_index_and_wordlist_directory	= "/home1/grupef/TopicBasedClusters_100_2_Indexes"
index_file								= "/home1/sengor/CLUEWEB/createIIS/merged_entry.txt"
word_list_file							= "/home1/sengor/CLUEWEB/createIIS/merged_wordlist.txt"

print "Script Started at: " + time.strftime('%X %x')

# WordList File iterator - Yields word and posting list length of word
def next_word_posting_list_length():
	with open(word_list_file) as infile:
		for line in infile:
			line_splitted = line.split(" ")
			yield (line_splitted[0],int(line_splitted[1]))

# Index File iterator - Yields doc_id,occurance_count and byte string to write clusters file
def bytes_from_file(filename, chunksize=8192):
	with open(filename, "rb") as f:
		while True:
			chunk = f.read(chunksize)
			# print pympler.asizeof.asizeof(chunk)
			print "New 1GB just read!"
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

def read_cluster_concat():
	
	cluster_concat_file = open(os.path.join(clusters_directory,"read","final","cluster_concat_sorted")) 
	cluster_concat_file_read = cluster_concat_file.readlines()
	cluster_concat_file.close()

	to_return = [[],[]]
	for index in xrange(len(cluster_concat_file_read)):
		line_splitted = cluster_concat_file_read[index].split(" ")
		to_return[0].append(int(line_splitted[0]))
		to_return[1].append(int(line_splitted[1]))

	return to_return


doc_ids = read_cluster_concat()
next_word_posting_list_length_getter = next_word_posting_list_length()

cluster_binary_files = []
cluster_wordlist_files = []
for i in xrange(__NO_OF_CLUSTERS__):
	cluster_binary_files.append(open(os.path.join(clusters_index_and_wordlist_directory,"merged_entry_" + str(i) + ".txt"),"wb"))
	cluster_wordlist_files.append(open(os.path.join(clusters_index_and_wordlist_directory,"merged_wordlist_" + str(i) + ".txt"),"w"))


cluster_binaries = []
cluster_word_lists = []
cluster_word_occurances = []
for i in xrange(__NO_OF_CLUSTERS__):
	cluster_binaries.append([])
	cluster_word_lists.append([])
	cluster_word_occurances.append(0)


(latest_word,posting_list_length) = next_word_posting_list_length_getter.next()
total_size_of_posting_lists_in_memory = posting_list_length
for (doc_id,occurance_count,byte_string) in bytes_from_file(index_file,10*1024*1024*1024): # 10GB per read!
	
	if posting_list_length == 0:
		for i in xrange(__NO_OF_CLUSTERS__):
			if cluster_word_occurances[i] > 0:
				cluster_word_lists[i].append(latest_word + " " + str(cluster_word_occurances[i]) + "\n")
				cluster_word_occurances[i] = 0

		if total_size_of_posting_lists_in_memory * 8 > 5*1024*1024*1024: # Larger than 5GB data in memory!
			print "Writing Started"
			for i in xrange(__NO_OF_CLUSTERS__):
				for binary_string in cluster_binaries[i]:
					cluster_binary_files[i].write(binary_string)
				for word_list_string in cluster_word_lists[i]:
					cluster_wordlist_files[i].write(word_list_string)
			cluster_binaries = []
			cluster_word_lists = []
			for i in xrange(__NO_OF_CLUSTERS__):
				cluster_binaries.append([])
				cluster_word_lists.append([])

			total_size_of_posting_lists_in_memory = 0
			print "Writing Ended"

		(latest_word,posting_list_length) = next_word_posting_list_length_getter.next()
		total_size_of_posting_lists_in_memory += posting_list_length
		

	posting_list_length = posting_list_length - 1

	cluster_id = doc_ids[1][doc_id - 1]

	cluster_binaries[cluster_id].append(byte_string)
	cluster_word_occurances[cluster_id] += 1


print "Last Writing Started"
for i in xrange(__NO_OF_CLUSTERS__):
	for binary_string in cluster_binaries[i]:
		cluster_binary_files[i].write(binary_string)
	for word_list_string in cluster_word_lists[i]:
		cluster_wordlist_files[i].write(word_list_string)
cluster_binaries = []
cluster_word_lists = []
print "Last Writing Ended"
for i in xrange(__NO_OF_CLUSTERS__):
	cluster_binary_files[i].close()
	cluster_wordlist_files[i].close()

print "Script ended at:   " + time.strftime('%X %x')
