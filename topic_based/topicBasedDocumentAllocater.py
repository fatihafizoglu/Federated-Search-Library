import sys
import os
import re
import random
import pickle
import math
from nltk.stem import PorterStemmer
from bs4 import BeautifulSoup  

__location__ = os.path.realpath(
	os.path.join(os.getcwd(), os.path.dirname(__file__)))

ARGV_INPUT_DIRECTORY = sys.argv[1]
ARGV_OUTPUT_DIRECTORY = sys.argv[2]
if os.path.exists(ARGV_OUTPUT_DIRECTORY):
	print "cannot create output directory '" + ARGV_OUTPUT_DIRECTORY + "': Directory exists"
	sys.exit()
ARGV_NO_OF_FILES = int(sys.argv[3])
ARGV_SAMPLE_PERCENTAGE = int(sys.argv[4])
ARGV_NO_OF_COLLECTION = int(sys.argv[5])
NO_OF_FILES_TO_BE_SAMPLED = (ARGV_NO_OF_FILES * ARGV_SAMPLE_PERCENTAGE) / 100
ARGV_LAMDBA = 0.1 # TODO: get as argument
ARGV_NO_OF_KMEANS_RUN = 5 # TODO: get as argument

class Collections:
	def __init__(self,collections_directory,no_of_collection):
		self.collections_directory = collections_directory
		self.no_of_collection = no_of_collection
		self.word_frequencies = []
		self.avarage_word_frequencies = {}
		self.stemmer = PorterStemmer()

		self.avarage_word_frequencies["total_word_count"] = 0

	def create_word_frequencies(self):
		for i in range(self.no_of_collection):
			try:
				collection_file = open(self.collections_directory + "/" + str(i) + "/1","r")
				collection_file_read = collection_file.read()
				collection_file.close()
			except IOError:
				collection_file_read = ""

			word_freq = {}
			word_freq["total_word_count"] = 0
			for document in re.findall("<DOC>.*?</DOC>",collection_file_read,re.S | re.I):

				document = re.sub(re.compile("<DOCNO>(.*?)</DOCHDR>",re.S | re.I | re.M),"",document)
				
				doc_soup = BeautifulSoup(document)
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

	def get_word_frequency_from_collecion(self,collection_id,word):
		try:
			return self.word_frequencies[collection_id][word]
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
# Get sample subset and save documents to sampled_docs file
def sample_documents():
	sampled_documents_indexes = random.sample(range(ARGV_NO_OF_FILES),NO_OF_FILES_TO_BE_SAMPLED)
	sampled_documents_indexes.sort()

	doc_usage_map = {}
	current_doc = 0
	sampled_documents_file = open(__location__ + "/sampled_docs","w")
	for filename in os.listdir(ARGV_INPUT_DIRECTORY):
		
		gov_dat_partition = open(ARGV_INPUT_DIRECTORY + "/" + filename)
		gov_dat_partition_read = gov_dat_partition.read()
		gov_dat_partition.close()

		documents = re.findall("<DOC>.*?</DOC>[^\<]",gov_dat_partition_read,re.S)

		for document in documents:

			doc_id = re.findall("<DOCNO>(.*?)<\/DOCNO>",document)[0]

			if current_doc in sampled_documents_indexes:
				doc_usage_map[doc_id] = True
				sampled_documents_file.write(document)
			else:
				doc_usage_map[doc_id] = False
			current_doc = current_doc + 1
	sampled_documents_file.close()
	pickle.dump(doc_usage_map,open("doc_usage_map","wb"))

#####
# Distribute first k documents from sampled set to k collections
def distribute_sampled_documents():
	os.mkdir(ARGV_OUTPUT_DIRECTORY)
	for i in range(ARGV_NO_OF_COLLECTION):
		os.mkdir(ARGV_OUTPUT_DIRECTORY + "/" + str(i))

	sampled_documents_file = open(__location__ + "/sampled_docs","r")
	sampled_documents_file_read = sampled_documents_file.read()
	sampled_documents_file.close()
	sampled_documents = re.findall("<DOC>.*?</DOC>[^\<]",sampled_documents_file_read,re.S)
	
	for index,sampled_document in enumerate(sampled_documents):
		if index < ARGV_NO_OF_COLLECTION:
			new_collection_file = open(ARGV_OUTPUT_DIRECTORY + "/" + str(index) + "/1","a")
			new_collection_file.write(sampled_document)
			new_collection_file.close()

#####
# K-means run
def run_kmeans():
	collections = Collections(ARGV_OUTPUT_DIRECTORY,ARGV_NO_OF_COLLECTION) 
	collections.create_word_frequencies()

	for i in range(ARGV_NO_OF_COLLECTION):
		os.remove(ARGV_OUTPUT_DIRECTORY + "/" + str(i) + "/1")

	sampled_documents_file = open(__location__ + "/sampled_docs","r")
	sampled_documents_file_read = sampled_documents_file.read()
	sampled_documents_file.close()
	sampled_documents = re.findall("<DOC>.*?</DOC>[^\<]",sampled_documents_file_read,re.S)

	for index,sampled_document in enumerate(sampled_documents):
		document = Document(sampled_document)
		
		max_sim_value = 0.0
		latest_collection_max_sim_obtained = 0
		
		for i in range(collections.no_of_collection):
			similarity_value = 0.0
			for word,freq in document.get_word_frequencies().items():
				if (collections.get_word_frequency_from_collecion(i,word) != 0):
					pciw = float(collections.get_word_frequency_from_collecion(i,word))/float(collections.get_word_frequency_from_collecion(i,"total_word_count"))
					pbw = float(collections.get_average_word_frequency(word))
					pdw = (1.0 - ARGV_LAMDBA) * (float(freq)/float(document.total_word_count)) + (ARGV_LAMDBA * float(pbw))
					similarity_value = similarity_value + (pciw * math.log10(pdw/(ARGV_LAMDBA*pbw)))
					similarity_value = similarity_value + (pdw * math.log10(pciw/(ARGV_LAMDBA*pbw)))
			if i==0:
				max_sim_value = similarity_value
				latest_collection_max_sim_obtained = i
			elif similarity_value > max_sim_value:
				max_sim_value = similarity_value
				latest_collection_max_sim_obtained = i

		new_collection_file = open(ARGV_OUTPUT_DIRECTORY + "/" + str(latest_collection_max_sim_obtained) + "/1","a")
		new_collection_file.write(document.content)
		new_collection_file.close()

sample_documents()
distribute_sampled_documents()

for kmeans_run in range(ARGV_NO_OF_KMEANS_RUN):
	run_kmeans()
	print "K-means run: " + str(kmeans_run + 1) + " ended"

# doc_usage_map = pickle.load( open( "doc_usage_map", "rb" ) )









# collections = Collections(ARGV_OUTPUT_DIRECTORY,ARGV_NO_OF_COLLECTION) 
# collections.create_word_frequencies()
# for i in range(100):
# 	print collections.get_word_frequency_from_collecion(i,"and")
# print collections.get_average_word_frequency("and")


# for i in range(100):
# 	print collections.get_word_frequency_from_collecion(i,"total_word_count")
# print collections.get_average_word_frequency("total_word_count")
