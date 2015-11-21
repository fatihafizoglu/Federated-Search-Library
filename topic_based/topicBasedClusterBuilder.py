import sys
import os
import re
import random
import math
import json
import time
from nltk.stem import PorterStemmer
from bs4 import BeautifulSoup  

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

print "Script started at: " + time.strftime('%X %x')

ARGV_INPUT_DIRECTORY = sys.argv[1]
ARGV_OUTPUT_DIRECTORY = sys.argv[2]
ARGV_NO_OF_CLUSTER = int(sys.argv[3])
ARGV_MAX_NO_OF_DOCUMENT_IN_A_FILE = int(sys.argv[4])
ARGV_NO_OF_DOCUMENTS_IN_COLLECTION = int(sys.argv[5])
ARGV_SAMPLE_PERCENTAGE = int(sys.argv[6])
NO_OF_FILES_TO_BE_SAMPLED = (ARGV_NO_OF_DOCUMENTS_IN_COLLECTION * ARGV_SAMPLE_PERCENTAGE) / 100

ARGV_LAMDBA = 0.1 # TODO: get as argument
ARGV_NO_OF_KMEANS_RUN = 5 # TODO: get as argument

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
		self.clusters_directory = ARGV_OUTPUT_DIRECTORY
		self.no_of_cluster = ARGV_NO_OF_CLUSTER
		self.word_frequencies = []
		self.stemmer = PorterStemmer()

		self.avarage_word_frequencies = {}
		self.avarage_word_frequencies["total_word_count"] = 0

	def create_word_frequencies(self):
		for i in range(self.no_of_cluster):
			word_freq = {}
			word_freq["total_word_count"] = 0

			for cluster_partition_name in os.listdir(os.path.join(self.clusters_directory,str(i))):
				try:
					cluster_partition = open(os.path.join(self.clusters_directory,str(i),cluster_partition_name),"r")
					cluster_partition_read = cluster_partition.read()
					cluster_partition.close()
				except IOError:
					cluster_partition_read = ""

				documents = re.findall("<DOC>.*?</DOC>",cluster_partition_read,re.S | re.I)
				
				for document in documents:

					document_content = re.sub(re.compile("<DOCNO>(.*?)</DOCHDR>",re.S | re.I | re.M),"",document)
				
					doc_soup = BeautifulSoup(document_content)
					[s.extract() for s in doc_soup(['style', 'script'])]
					document_visible_text = doc_soup.getText()

					word_list = document_visible_text.split()
					for word in word_list:
						word_stemmed = self.stemmer.stem(word).lower()
						if word_freq.has_key(word_stemmed):
							word_freq[word_stemmed] = word_freq[word_stemmed] + 1
						else:
							word_freq[word_stemmed] = 1
						if self.avarage_word_frequencies.has_key(word_stemmed):
							self.avarage_word_frequencies[word_stemmed] = self.avarage_word_frequencies[word_stemmed] + 1
						else:
							self.avarage_word_frequencies[word_stemmed] = 1

						word_freq["total_word_count"] = word_freq["total_word_count"] + 1
						self.avarage_word_frequencies["total_word_count"] = self.avarage_word_frequencies["total_word_count"] + 1
			self.word_frequencies.append(word_freq)

	def get_word_frequency_from_cluster(self,cluster_id,word):
		try:
			return self.word_frequencies[cluster_id][word]
		except KeyError:
			return 0
	def get_average_word_frequency(self,word):
		try:
			return float(self.avarage_word_frequencies[word]) / float(self.avarage_word_frequencies["total_word_count"])
		except KeyError:
			return 0

class Document:
	def __init__(self,content):
		self.content = content
		self.word_frequencies = {}
		self.total_word_count = 0
		self.stemmer = PorterStemmer()

		self.create_word_frequencies()

	def create_word_frequencies(self):
		document = re.sub(re.compile("<DOCNO>(.*?)</DOCHDR>",re.S | re.I | re.M),"",self.content)
		
		doc_soup = BeautifulSoup(document)
		[s.extract() for s in doc_soup(['style', 'script'])]
		document_visible_text = doc_soup.getText()

		word_list = document_visible_text.split()
		for word in word_list:
			word_stemmed = self.stemmer.stem(word).lower()
			if self.word_frequencies.has_key(word_stemmed):
				self.word_frequencies[word_stemmed] = self.word_frequencies[word_stemmed] + 1
			else:
				self.word_frequencies[word_stemmed] = 1
			self.total_word_count = self.total_word_count + 1

	def get_word_frequencies(self):
		return self.word_frequencies

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

			documents = re.findall("<DOC>.*?</DOC>[^\<]",gov_dat_partition_read,re.S)

			for document in documents:

				doc_id = re.findall("<DOCNO>(.*?)<\/DOCNO>",document)[0]

				if current_document_index in document_indexes_to_sample:
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

		documents = re.findall("<DOC>.*?</DOC>[^\<]",sampled_documents_partition_read,re.S)

		for document in documents:

			doc_id = re.findall("<DOCNO>(.*?)<\/DOCNO>",document)[0]

			if current_document_index in document_indexes_to_random_distribution:
				documents_used_in_random_distribution["documents"].append(doc_id)
				cluster_to_insert_document = len(documents_used_in_random_distribution["documents"]) - 1

				cluster_documents_file = open(os.path.join(ARGV_OUTPUT_DIRECTORY,str(cluster_to_insert_document),"0"),"w")
				cluster_documents_file.write(document)
				cluster_documents_file.close()

			current_document_index = current_document_index + 1

	with open(os.path.join(ARGV_OUTPUT_DIRECTORY,"SampledDocuments","documents_used_in_random_distribution.json"), "w") as fp:
		json.dump(documents_used_in_random_distribution, fp)

#####
# K-means run
def run_kmeans():
	clusters = Clusters() 
	clusters.create_word_frequencies()

	for i in range(ARGV_NO_OF_CLUSTER):
		for cluster_partition in os.listdir(os.path.join(ARGV_OUTPUT_DIRECTORY,str(i))):
			os.remove(os.path.join(ARGV_OUTPUT_DIRECTORY,str(i),cluster_partition))

	cluster_size_dict = {}
	for sampled_document_file_name in os.listdir(os.path.join(ARGV_OUTPUT_DIRECTORY,"SampledDocuments")):
		if ".json" in sampled_document_file_name: # For skipping json files created in same directory
			continue

		sampled_documents_partition = open(os.path.join(ARGV_OUTPUT_DIRECTORY,"SampledDocuments",sampled_document_file_name),"r")
		sampled_documents_partition_read = sampled_documents_partition.read()
		sampled_documents_partition.close()

		documents = re.findall("<DOC>.*?</DOC>[^\<]",sampled_documents_partition_read,re.S)

		for document in documents:
			document_instance = Document(document)

			maximum_similarity_value = 0
			latest_cluster_maximum_similarity_value_obtained = 0

			for i in range(ARGV_NO_OF_CLUSTER):
				similarity_value = 0.0
				for word,freq in document_instance.get_word_frequencies().items():
					if (clusters.get_word_frequency_from_cluster(i,word) != 0):
						pciw = float(clusters.get_word_frequency_from_cluster(i,word))/float(clusters.get_word_frequency_from_cluster(i,"total_word_count"))
						pbw = float(clusters.get_average_word_frequency(word))
						pdw = (1.0 - ARGV_LAMDBA) * (float(freq)/float(document_instance.total_word_count)) + (ARGV_LAMDBA * float(pbw))
						similarity_value = similarity_value + (pciw * math.log10(pdw/(ARGV_LAMDBA*pbw)))
						similarity_value = similarity_value + (pdw * math.log10(pciw/(ARGV_LAMDBA*pbw)))
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
			cluster_documents_file.write(document_instance.content)
			cluster_documents_file.close()

sample_documents()
select_pivot_points_and_copy_to_clusters()



for kmeans_run in range(ARGV_NO_OF_KMEANS_RUN):
	run_kmeans()
	print "K-means run: " + str(kmeans_run + 1) + " ended"

# # #
# # TEST 1
# clusters = Clusters() 
# clusters.create_word_frequencies()
# for i in range(50):
# 	print clusters.get_word_frequency_from_cluster(i,"total_word_count")
# print clusters.get_average_word_frequency("total_word_count")

# # #
# # TEST 2
# documents_used_in_random_distribution = {}
# with open(os.path.join(ARGV_OUTPUT_DIRECTORY,"SampledDocuments","documents_used_in_random_distribution.json")) as fp:
# 	documents_used_in_random_distribution = json.load(fp)

# clusters = Clusters() 
# clusters.create_word_frequencies()

# for sampled_document_file_name in os.listdir(os.path.join(ARGV_OUTPUT_DIRECTORY,"SampledDocuments")):
# 	if ".json" in sampled_document_file_name: # For skipping json files created in same directory
# 		continue

# 	sampled_documents_partition = open(os.path.join(ARGV_OUTPUT_DIRECTORY,"SampledDocuments",sampled_document_file_name),"r")
# 	sampled_documents_partition_read = sampled_documents_partition.read()
# 	sampled_documents_partition.close()

# 	documents = re.findall("<DOC>.*?</DOC>[^\<]",sampled_documents_partition_read,re.S)

# 	for document in documents:

# 		doc_id = re.findall("<DOCNO>(.*?)<\/DOCNO>",document)[0]

# 		if doc_id in documents_used_in_random_distribution["documents"]:

# 			document_instance = Document(document)

# 			maximum_similarity_value = 0
# 			latest_cluster_maximum_similarity_value_obtained = 0

# 			for i in range(ARGV_NO_OF_CLUSTER):
# 				similarity_value = 0.0
# 				for word,freq in document_instance.get_word_frequencies().items():
# 					if (clusters.get_word_frequency_from_cluster(i,word) != 0):
# 						pciw = float(clusters.get_word_frequency_from_cluster(i,word))/float(clusters.get_word_frequency_from_cluster(i,"total_word_count"))
# 						pbw = float(clusters.get_average_word_frequency(word))
# 						pdw = (1.0 - ARGV_LAMDBA) * (float(freq)/float(document_instance.total_word_count)) + (ARGV_LAMDBA * float(pbw))
# 						similarity_value = similarity_value + (pciw * math.log10(pdw/(ARGV_LAMDBA*pbw)))
# 						similarity_value = similarity_value + (pdw * math.log10(pciw/(ARGV_LAMDBA*pbw)))
# 				if i==0 or similarity_value > maximum_similarity_value:
# 					maximum_similarity_value = similarity_value
# 					latest_collection_max_sim_obtained = i
# 			print "************************************************************"
# 			print doc_id
# 			print latest_collection_max_sim_obtained
# 			print "************************************************************"

print "Script ended at:   " + time.strftime('%X %x')