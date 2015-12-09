#-*- coding: utf-8 -*-
import sys
import os
import re
import random
import math
import json
import time
from nltk.stem import PorterStemmer
import lxml
from lxml.html.clean import Cleaner

def print_usage():
	print "\nWRONG USAGE:"
	print "Usage: python topicBasedClusterBuilder.py [IDP] [ODP] [NOC] [MS] [NOD] [SP]"
	print "\nArguments"
	print "\t IDP           Input Directory Path"
	print "\t ODP           Output Directory Path"
	print "\t NOC           Number of Clusters"
	print "\t MS            Maximum number of document in one large file"
	print "\t NOD           Number of documents in collection"
	print "\t SP            Percentage of sample"
	print "Ex:   python topicBasedClusterBuilder.py /media/arcelik/UbuntuExtra/govDatExtracted /media/arcelik/UbuntuExtra/TopicBasedClusters 50 1000 89771 1"

if len(sys.argv) < 7:
	print_usage()
	sys.exit()

print "Script started at        : " + time.strftime('%X %x')

ARGV_INPUT_DIRECTORY = sys.argv[1]
ARGV_OUTPUT_DIRECTORY = sys.argv[2]
ARGV_NO_OF_CLUSTER = int(sys.argv[3])
ARGV_MAX_NO_OF_DOCUMENT_IN_A_FILE = int(sys.argv[4])
ARGV_NO_OF_DOCUMENTS_IN_COLLECTION = int(sys.argv[5])
ARGV_SAMPLE_PERCENTAGE = int(sys.argv[6])
NO_OF_FILES_TO_BE_SAMPLED = (ARGV_NO_OF_DOCUMENTS_IN_COLLECTION * ARGV_SAMPLE_PERCENTAGE) / 100

ARGV_LAMDBA = 0.1 # TODO: get as argument
ARGV_NO_OF_KMEANS_RUN = 5 # TODO: get as argument

find_doc_pattern = re.compile("<DOC>.*?</DOC>[^\<]",re.S)
find_doc_id_pattern = re.compile("<DOCNO>(.*?)<\/DOCNO>")
find_doc_content_pattern = re.compile("<DOCNO>(.*?)</DOCHDR>",re.S | re.I | re.M)

if os.path.exists(ARGV_OUTPUT_DIRECTORY):
	print "cannot create output directory '" + ARGV_OUTPUT_DIRECTORY + "': Directory exists"
	sys.exit()
else:
	os.mkdir(ARGV_OUTPUT_DIRECTORY)
	for i in range(ARGV_NO_OF_CLUSTER):
		os.mkdir(os.path.join(ARGV_OUTPUT_DIRECTORY,str(i)))
	os.mkdir(os.path.join(ARGV_OUTPUT_DIRECTORY,"SampledDocuments"))

class Clusters:
	
	def __init__(self):

		self.word_frequencies = []
		self.avarage_word_frequencies = {}

		self.new_word_frequencies = []
		for i in range(ARGV_NO_OF_CLUSTER):
			self.new_word_frequencies.append({"total_word_count" : 0})
		self.new_avarage_word_frequencies = {"total_word_count" : 0}

	def add_new_document_to_new_word_frequencies(self,document,cluster_id):

		for word,freq in document.word_frequencies.items():
			try:
				self.new_word_frequencies[cluster_id][word] = self.new_word_frequencies[cluster_id][word] + freq
			except:
				self.new_word_frequencies[cluster_id][word] = freq
			try:
				self.new_avarage_word_frequencies[word] = self.new_avarage_word_frequencies[word] + freq
			except:
				self.new_avarage_word_frequencies[word] = freq

		self.new_word_frequencies[cluster_id]["total_word_count"] = self.new_word_frequencies[cluster_id]["total_word_count"] + document.total_word_count
		self.new_avarage_word_frequencies["total_word_count"] = self.new_avarage_word_frequencies["total_word_count"] + document.total_word_count
		

	def update_word_frequencies(self):

		self.word_frequencies = self.new_word_frequencies
		self.avarage_word_frequencies = self.new_avarage_word_frequencies

		self.new_word_frequencies = []
		for i in range(ARGV_NO_OF_CLUSTER):
			self.new_word_frequencies.append({"total_word_count" : 0})
		self.new_avarage_word_frequencies = {"total_word_count" : 0}

class Document:
	
	def __init__(self,content):

		self.content = content
		self.word_frequencies = {}
		self.total_word_count = 0
		# self.stemmer = PorterStemmer()

	def create_word_frequencies(self):
		
		document = re.sub(find_doc_content_pattern,"",self.content)
		
		cleaner = Cleaner()
		cleaner.scripts = True
		cleaner.javascript = True
		cleaner.style = True
		# # cleaner.allow_tags = ['']
		# # cleaner.remove_unknown_tags = False

		try:
			document_visible_text = cleaner.clean_html(document)
		except UnicodeDecodeError:
			document_visible_text = ""
			print "Unicode Error"
		# document_visible_text = document

		word_list = document_visible_text.split()
		for word in word_list:
			word_stemmed = word.lower()
			try:
				self.word_frequencies[word_stemmed] = self.word_frequencies[word_stemmed] + 1
			except:
				self.word_frequencies[word_stemmed] = 1
			self.total_word_count = self.total_word_count + 1

#####
# Get sample subset and save documents to sampled_docs files under output directory
def sample_documents():
	
	document_indexes_to_sample = random.sample(range(ARGV_NO_OF_DOCUMENTS_IN_COLLECTION),NO_OF_FILES_TO_BE_SAMPLED)
	document_indexes_to_sample.sort()

	documents_used_in_sampling = {"documents":[]}

	latest_opened_sampled_document_file_name = 0
	latest_opened_sampled_documents_file = open(os.path.join(ARGV_OUTPUT_DIRECTORY,"SampledDocuments",str(latest_opened_sampled_document_file_name)),"w")
	number_of_sampled_documents = 0

	current_document_index = 0
	for directory_name in os.listdir(ARGV_INPUT_DIRECTORY):
		for file_name in os.listdir(os.path.join(ARGV_INPUT_DIRECTORY,directory_name)):
			if ".gz" in file_name: # For skipping zipped files
				continue

			gov_dat_partition = open(os.path.join(ARGV_INPUT_DIRECTORY,directory_name,file_name))
			gov_dat_partition_read = gov_dat_partition.read()
			gov_dat_partition.close()

			for document in re.findall(find_doc_pattern,gov_dat_partition_read):


				if current_document_index in document_indexes_to_sample:
					doc_id = re.findall(find_doc_id_pattern,document)[0]
					
					documents_used_in_sampling["documents"].append(doc_id)
					latest_opened_sampled_documents_file.write(document)

					number_of_sampled_documents = number_of_sampled_documents + 1
					if (number_of_sampled_documents / ARGV_MAX_NO_OF_DOCUMENT_IN_A_FILE) > latest_opened_sampled_document_file_name:
						latest_opened_sampled_documents_file.close()
						latest_opened_sampled_document_file_name = latest_opened_sampled_document_file_name + 1
						latest_opened_sampled_documents_file = open(os.path.join(ARGV_OUTPUT_DIRECTORY,"SampledDocuments",str(latest_opened_sampled_document_file_name)),"w")
				
				current_document_index = current_document_index + 1

	latest_opened_sampled_documents_file.close()
	with open(os.path.join(ARGV_OUTPUT_DIRECTORY,"SampledDocuments","documents_used_in_sampling.json"), "w") as fp:
		json.dump(documents_used_in_sampling, fp)

#####
# Distribute random k documents from sampled set to k clusters, which used as pivot points later
def select_pivot_points_and_copy_to_clusters():

	clusters = Clusters()

	document_indexes_to_random_distribution = random.sample(range(NO_OF_FILES_TO_BE_SAMPLED),ARGV_NO_OF_CLUSTER)
	document_indexes_to_random_distribution.sort()

	documents_used_in_random_distribution = {"documents":[]}

	current_document_index = 0
	for sampled_document_file_name in os.listdir(os.path.join(ARGV_OUTPUT_DIRECTORY,"SampledDocuments")):
		if ".json" in sampled_document_file_name: # For skipping json files created in same directory
			continue

		sampled_documents_partition = open(os.path.join(ARGV_OUTPUT_DIRECTORY,"SampledDocuments",sampled_document_file_name))
		sampled_documents_partition_read = sampled_documents_partition.read()
		sampled_documents_partition.close()


		for document in re.findall(find_doc_pattern,sampled_documents_partition_read):

			if current_document_index in document_indexes_to_random_distribution:
				doc_id = re.findall(find_doc_id_pattern,document)[0]
				
				documents_used_in_random_distribution["documents"].append(doc_id)
				cluster_to_insert_document = len(documents_used_in_random_distribution["documents"]) - 1

				document_instance = Document(document)
				document_instance.create_word_frequencies()
				clusters.add_new_document_to_new_word_frequencies(document_instance,cluster_to_insert_document)

			current_document_index = current_document_index + 1

	with open(os.path.join(ARGV_OUTPUT_DIRECTORY,"SampledDocuments","documents_used_in_random_distribution.json"), "w") as fp:
		json.dump(documents_used_in_random_distribution, fp)

	return clusters

sample_documents()

print "Sample Documents ended at: " + time.strftime('%X %x')

clusters = select_pivot_points_and_copy_to_clusters()
clusters.update_word_frequencies()
#####
# K-means run
# @profile
def run_kmeans():
	global clusters
	cluster_word_frequencies = clusters.word_frequencies
	cluster_avg_word_frequencies = clusters.avarage_word_frequencies
	cluster_avg_word_frequencies_total_inv = 1/float(cluster_avg_word_frequencies["total_word_count"])
	for sampled_document_file_name in os.listdir(os.path.join(ARGV_OUTPUT_DIRECTORY,"SampledDocuments")):
		if ".json" in sampled_document_file_name: # For skipping json files created in same directory
			continue

		sampled_documents_partition = open(os.path.join(ARGV_OUTPUT_DIRECTORY,"SampledDocuments",sampled_document_file_name),"r")
		sampled_documents_partition_read = sampled_documents_partition.read()
		sampled_documents_partition.close()


		for document in re.findall(find_doc_pattern,sampled_documents_partition_read):
			document_instance = Document(document)
			document_instance.create_word_frequencies()

			# document_word_frequencies_items = document_instance.word_frequencies.items()
			document_word_frequencies = document_instance.word_frequencies
			try:
				document_word_frequencies_total_inv = 1/float(document_instance.total_word_count)
			except ZeroDivisionError:
				doc_id = re.findall("<DOCNO>(.*?)<\/DOCNO>",document)[0]
				print "Document with Zero word!"
				print doc_id
				continue

			maximum_similarity_value = 0
			latest_cluster_maximum_similarity_value_obtained = 0

			for i in range(ARGV_NO_OF_CLUSTER):
				cluster_word_frequency = cluster_word_frequencies[i]
				cluster_word_frequency_total_inv = 1/float(cluster_word_frequency["total_word_count"])
				similarity_value = 0.0
				for word,freq in document_word_frequencies.iteritems():
					# try:
						# if (cluster_word_frequency[word] != 0):
					if word in cluster_word_frequency:
						pciw = cluster_word_frequency[word] * cluster_word_frequency_total_inv
						pbw = cluster_avg_word_frequencies[word] * cluster_avg_word_frequencies_total_inv
						pdw = (1.0 - ARGV_LAMDBA) * (freq * document_word_frequencies_total_inv) + (ARGV_LAMDBA * pbw)
						similarity_value += (pciw * math.log10(pdw/(ARGV_LAMDBA*pbw))) + (pdw * math.log10(pciw/(ARGV_LAMDBA*pbw)))
					# except KeyError:
						# continue
				if i==0 or similarity_value > maximum_similarity_value:
					maximum_similarity_value = similarity_value
					latest_cluster_maximum_similarity_value_obtained = i

			clusters.add_new_document_to_new_word_frequencies(document_instance,latest_cluster_maximum_similarity_value_obtained)

	# return clusters

# @profile
def after_kmeans():
	global clusters
	cluster_word_frequencies = clusters.word_frequencies
	cluster_avg_word_frequencies = clusters.avarage_word_frequencies
	cluster_avg_word_frequencies_total_inv = 1/float(cluster_avg_word_frequencies["total_word_count"])
	
	cluster_size_dict = {}
	for directory_name in os.listdir(ARGV_INPUT_DIRECTORY):
		for file_name in os.listdir(os.path.join(ARGV_INPUT_DIRECTORY,directory_name)):
			if ".gz" in file_name: # For skipping zipped files
				continue

			gov_dat_partition = open(os.path.join(ARGV_INPUT_DIRECTORY,directory_name,file_name))
			gov_dat_partition_read = gov_dat_partition.read()
			gov_dat_partition.close()

			for document in re.findall(find_doc_pattern,gov_dat_partition_read):
				document_instance = Document(document)
				document_instance.create_word_frequencies()

				# document_word_frequencies_items = document_instance.word_frequencies.items()
				document_word_frequencies = document_instance.word_frequencies
				try:
					document_word_frequencies_total_inv = 1/float(document_instance.total_word_count)
				except ZeroDivisionError:
					doc_id = re.findall("<DOCNO>(.*?)<\/DOCNO>",document)[0]
					print "Document with Zero word!"
					print doc_id
					continue

				maximum_similarity_value = 0
				latest_cluster_maximum_similarity_value_obtained = 0

				for i in range(ARGV_NO_OF_CLUSTER):
					cluster_word_frequency = cluster_word_frequencies[i]
					cluster_word_frequency_total_inv = 1/float(cluster_word_frequency["total_word_count"])
					similarity_value = 0.0
					for word,freq in document_word_frequencies.iteritems():
						# try:
							# if (cluster_word_frequency[word] != 0):
						if word in cluster_word_frequency:
							pciw = cluster_word_frequency[word] * cluster_word_frequency_total_inv
							pbw = cluster_avg_word_frequencies[word] * cluster_avg_word_frequencies_total_inv
							pdw = (1.0 - ARGV_LAMDBA) * (freq * document_word_frequencies_total_inv) + (ARGV_LAMDBA * pbw)
							similarity_value += (pciw * math.log10(pdw/(ARGV_LAMDBA*pbw))) + (pdw * math.log10(pciw/(ARGV_LAMDBA*pbw)))
						# except KeyError:
							# continue
					if i==0 or similarity_value > maximum_similarity_value:
						maximum_similarity_value = similarity_value
						latest_cluster_maximum_similarity_value_obtained = i

				output_file_name = 0
				if cluster_size_dict.has_key(latest_cluster_maximum_similarity_value_obtained):
					output_file_name = cluster_size_dict[latest_cluster_maximum_similarity_value_obtained] / ARGV_MAX_NO_OF_DOCUMENT_IN_A_FILE
					cluster_size_dict[latest_cluster_maximum_similarity_value_obtained] = cluster_size_dict[latest_cluster_maximum_similarity_value_obtained] + 1
				else:
					cluster_size_dict[latest_cluster_maximum_similarity_value_obtained] = 1

				cluster_documents_file = open(os.path.join(ARGV_OUTPUT_DIRECTORY,str(latest_cluster_maximum_similarity_value_obtained),str(output_file_name)),"a")
				cluster_documents_file.write(document)
				cluster_documents_file.close()
			print "Partition: " + directory_name + "/" + file_name + " ended at  : " + time.strftime('%X %x')
		break

for kmeans_run in range(ARGV_NO_OF_KMEANS_RUN):
	# clusters = run_kmeans(clusters)
	run_kmeans()
	clusters.update_word_frequencies()
	print "K-means run: " + str(kmeans_run + 1) + " ended at  : " + time.strftime('%X %x')

after_kmeans()

print "Script ended at          : " + time.strftime('%X %x')

















# #
# TEST 2
documents_used_in_random_distribution = {}
with open(os.path.join(ARGV_OUTPUT_DIRECTORY,"SampledDocuments","documents_used_in_random_distribution.json")) as fp:
	documents_used_in_random_distribution = json.load(fp)

cluster_word_frequencies = clusters.word_frequencies
cluster_avg_word_frequencies = clusters.avarage_word_frequencies
cluster_avg_word_frequencies_total = float(cluster_avg_word_frequencies["total_word_count"])
for sampled_document_file_name in os.listdir(os.path.join(ARGV_OUTPUT_DIRECTORY,"SampledDocuments")):
	if ".json" in sampled_document_file_name: # For skipping json files created in same directory
		continue

	sampled_documents_partition = open(os.path.join(ARGV_OUTPUT_DIRECTORY,"SampledDocuments",sampled_document_file_name),"r")
	sampled_documents_partition_read = sampled_documents_partition.read()
	sampled_documents_partition.close()


	for document in re.findall(find_doc_pattern,sampled_documents_partition_read):

		doc_id = re.findall("<DOCNO>(.*?)<\/DOCNO>",document)[0]

		if doc_id in documents_used_in_random_distribution["documents"]:

			document_instance = Document(document)
			document_instance.create_word_frequencies()

			# document_word_frequencies_items = document_instance.get_word_frequencies().items()
			document_word_frequencies = document_instance.word_frequencies
			document_word_frequencies_total = float(document_instance.total_word_count)

			maximum_similarity_value = 0
			latest_cluster_maximum_similarity_value_obtained = 0

			for i in range(ARGV_NO_OF_CLUSTER):
				cluster_word_frequency = cluster_word_frequencies[i]
				cluster_word_frequency_total = float(cluster_word_frequency["total_word_count"])
				similarity_value = 0.0
				for word,freq in document_word_frequencies.iteritems():
					# try:
						# if (cluster_word_frequency[word] != 0):
					if word in cluster_word_frequency:
						pciw = cluster_word_frequency[word]/cluster_word_frequency_total
						pbw = cluster_avg_word_frequencies[word]/cluster_avg_word_frequencies_total
						pdw = (1.0 - ARGV_LAMDBA) * (freq/document_word_frequencies_total) + (ARGV_LAMDBA * pbw)
						similarity_value += (pciw * math.log10(pdw/(ARGV_LAMDBA*pbw))) + (pdw * math.log10(pciw/(ARGV_LAMDBA*pbw)))
					# except KeyError:
						# continue
				if i==0 or similarity_value > maximum_similarity_value:
					maximum_similarity_value = similarity_value
					latest_cluster_maximum_similarity_value_obtained = i

			print latest_cluster_maximum_similarity_value_obtained