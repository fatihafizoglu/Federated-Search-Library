import os
import time
import struct
import math


print "Script started at: " + time.strftime('%X %x')

INTEGER_SIZE = 4

LAMBDA_ARRAY = [0.25,0.50,0.75,1.00]

DIV_SIZE = 200

WORD_NO = 163629158
wordlist_file = "/home1/grupef/ecank/data/wordlist_TOPIC100_CSI_IDF"
cfcweights = []


DOC_NUM = 50220538
smart_documents_file = "/home1/grupef/ecank/data/doc_lengths"
unique_terms = []
total_tf_per_doc = []


QUERY_NO = 198
query_results_file = "/home1/grupef/ecank/results/c1kns/c1kns_fixed"
query_results = []

doc_file_names = ["/home1/grupef/ecank/data/document_vectors/dvec.bin-1",
				  "/home1/grupef/ecank/data/document_vectors/dvec.bin-2",
				  "/home1/grupef/ecank/data/document_vectors/dvec.bin-3",
				  "/home1/grupef/ecank/data/document_vectors/dvec.bin-4",
				  "/home1/grupef/ecank/data/document_vectors/dvec.bin-5",
				  "/home1/grupef/ecank/data/document_vectors/dvec.bin-6",
				  "/home1/grupef/ecank/data/document_vectors/dvec.bin-7",
				  "/home1/grupef/ecank/data/document_vectors/dvec.bin-8",
				  "/home1/grupef/ecank/data/document_vectors/dvec.bin-9",
				  "/home1/grupef/ecank/data/document_vectors/dvec.bin-10",
				  "/home1/grupef/ecank/data/document_vectors/dvec.bin-11"
				]
doc_files = []

diversified_query_results_file = "output.txt"

def readWordList():
	global cfcweights
	cfcweights = [float(0) for i in range(WORD_NO + 1)]
	i = 1
	with open(wordlist_file) as f:
		for line in f:
			line_splitted = line.split(" ")
			cfcweights[i] = float(line_splitted[2])
			i = i + 1

def readSmartDocuments():
	global unique_terms
	global total_tf_per_doc
	unique_terms = [float(0) for i in range(DOC_NUM + 1)]
	total_tf_per_doc = [float(0) for i in range(DOC_NUM + 1)]
	i = 1
	with open(smart_documents_file) as f:
		for line in f:
			line_splitted = line.split(" ")
			unique_terms[i] = int(line_splitted[1])
			total_tf_per_doc[i] = int(line_splitted[2])
			i = i + 1

def readQueryResults():
	global query_results
	query_results = [[] for i in range(QUERY_NO + 1)]
	for query_result in query_results:
		query_result.append((-1,-1,-1))
	with open(query_results_file) as f:
		for line in f:
			line_splitted = line.split("\t")
			query_id = int(line_splitted[0])
			doc_id = int(line_splitted[2])
			doc_rank = int(line_splitted[3])
			doc_score = float(line_splitted[4])

			query_results[query_id].append((doc_id,doc_rank,doc_score))

def setAdresses(start,end):
	global unique_terms
	global doc_addresses

	for i in range(start,end):
		if i == start:
			doc_addresses[i] = 0
		else:
			doc_addresses[i] = doc_addresses[i-1] + INTEGER_SIZE * 2 * unique_terms[i-1]

def openDocFiles():
	global doc_file_names
	global doc_files
	global doc_addresses

	doc_addresses = [0 for i in range(DOC_NUM + 1)]

	for doc_file_name in doc_file_names:
		doc_file = open(doc_file_name,"rb")
		doc_files.append(doc_file)

	setAdresses(1,         5000000)
	setAdresses(5000000,  10000000)
	setAdresses(10000000, 15000000)
	setAdresses(15000000, 20000000)
	setAdresses(20000000, 25000000)
	setAdresses(25000000, 30000000)
	setAdresses(30000000, 35000000)
	setAdresses(35000000, 40000000)
	setAdresses(40000000, 45000000)
	setAdresses(45000000, 50000000)
	setAdresses(50000000, 50220539)

def getDocumentVector(doc_id):
	global doc_addresses
	global cfcweights

	file = None
	if doc_id >= 1 and doc_id < 5000000:
		file = doc_files[0]
	elif doc_id >= 5000000 and doc_id < 10000000:
		file = doc_files[1]
	elif doc_id >= 10000000 and doc_id <  15000000:
		file = doc_files[2]
	elif doc_id >= 15000000 and doc_id <  20000000:
		file = doc_files[3]
	elif doc_id >= 20000000 and doc_id <  25000000:
		file = doc_files[4]
	elif doc_id >= 25000000 and doc_id <  30000000:
		file = doc_files[5]
	elif doc_id >= 30000000 and doc_id <  35000000:
		file = doc_files[6]
	elif doc_id >= 35000000 and doc_id <  40000000:
		file = doc_files[7]
	elif doc_id >= 40000000 and doc_id <  45000000:
		file = doc_files[8]
	elif doc_id >= 45000000 and doc_id <  50000000:
		file = doc_files[9]
	elif doc_id >= 50000000 and doc_id <  50220539:
		file = doc_files[10]
	else:
		print "Something went wrong!! - getDocumentVector - Possible Invalid doc_id"

	doc_vector = []
	file.seek(doc_addresses[doc_id])
	chunk = file.read(unique_terms[doc_id] * 2 * INTEGER_SIZE)
	byte_string = ""
	for byte in chunk:
		byte_string += byte
		if len(byte_string) == INTEGER_SIZE * 2:
			dvec = struct.unpack("II", byte_string)
			doc_vector.append((dvec[0],dvec[1]))
			# doc_vector.append((dvec[0],cfcweights[dvec[0]] * dvec[1]))
			# if cfcweights[dvec[0]] <= 0:
			# 	print "Something went wrong!! - getDocumentVector - Unused term in document. Cannot happen`"
			byte_string = ""

	doc_vector_ = []
	total_tf = sum([dvec[1] for dvec in doc_vector])
	for dvec in doc_vector:
		doc_vector_.append((dvec[0],(float(dvec[1]) / total_tf) * cfcweights[dvec[0]]))
		if cfcweights[dvec[0]] <= 0:
			print "Something went wrong!! - getDocumentVector - Unused term in document. Cannot happen"


	return doc_vector_

def getVectorLength(doc_vector):

	summ = 0.0
	for dvec in doc_vector:
		summ += dvec[1] * dvec[1]
	return math.sqrt(summ)

def dotProduct(doc_vector1,doc_vector2,doc_vector1_length,doc_vector2_length):

	summ = 0.0
	i = 0
	j = 0

	while(i < len(doc_vector1) and j < len(doc_vector2)):
		if doc_vector1[i][0] == doc_vector2[j][0]:
			summ += doc_vector1[i][1] * doc_vector2[j][1]
			i += 1
			j += 1
		elif doc_vector1[i][0] < doc_vector2[j][0]:
			i += 1
		elif doc_vector1[i][0] > doc_vector2[j][0]:
			j += 1

	return summ / (doc_vector1_length * doc_vector2_length)


def diversifyMaxSum(query_id,_lambda_):
	global query_results

	diversified_query_result = []

	number_of_results = len(query_results[query_id])

	dvecs = [[] for i in range(number_of_results)]
	dvec_lengths = [1 for i in range(number_of_results)]
	for i in range(1,number_of_results):
		dvecs[i] = getDocumentVector(query_results[query_id][i][0])
		dvec_lengths[i] = getVectorLength(dvecs[i])

	distances = [[] for i in range(number_of_results)]
	for distance in distances:
		for i in range(number_of_results):
			distance.append(-1)

	maximum_relevance_score = 0.0
	relevance_score_sum = 0.0
	for (doc_id,doc_rank,doc_score) in query_results[query_id]:
		if doc_score > maximum_relevance_score:
			maximum_relevance_score = doc_score
		relevance_score_sum += doc_score

	for i in range(1,number_of_results):
		for j in range(i + 1,number_of_results):
			distances[i][j] = (1 - _lambda_) * ((query_results[query_id][i][2]/maximum_relevance_score) + (query_results[query_id][j][2]/maximum_relevance_score)) + 2 * _lambda_ * (1 - dotProduct(dvecs[i],dvecs[j],dvec_lengths[i],dvec_lengths[j]))

	k = 1
	while k <= DIV_SIZE / 2:
		max_dist = 0.0
		max_i = 0
		max_j = 0

		for i in range(1,number_of_results):
			for j in range(i + 1,number_of_results):
				if distances[i][j] > max_dist:
					max_dist = distances[i][j]
					max_i = i
					max_j = j

		# print max_i
		# print max_j

		if max_dist > 0:
			diversified_query_result.append((query_results[query_id][max_i][0],2 * k - 1,query_results[query_id][max_i][2]))
			diversified_query_result.append((query_results[query_id][max_j][0],2 * k,query_results[query_id][max_j][2]))


			for i in range(1,number_of_results):
				distances[i][max_i] = -1.0
				distances[i][max_j] = -1.0

				distances[max_i][i] = -1.0
				distances[max_j][i] = -1.0

		k += 1

	diversified_query_result.sort(key=lambda tup: tup[2],reverse=True)
	diversified_query_result.insert(0,(-1,-1,-1))

	return diversified_query_result

def diversifySy(query_id,_lambda_):
	global query_results

	diversified_query_result = []

	number_of_results = len(query_results[query_id])

	dvecs = [[] for i in range(number_of_results)]
	dvec_lengths = [1 for i in range(number_of_results)]
	for i in range(1,number_of_results):
		dvecs[i] = getDocumentVector(query_results[query_id][i][0])
		dvec_lengths[i] = getVectorLength(dvecs[i])


	diversified_query_result = query_results[query_id][:]
	i = 1
	while i <= DIV_SIZE and i < number_of_results:
		j = i + 1
		while j < len(diversified_query_result) and len(diversified_query_result) > DIV_SIZE + 1:
			if dotProduct(dvecs[i],dvecs[j],dvec_lengths[i],dvec_lengths[j]) > (1 - _lambda_):
				diversified_query_result.pop(j)
				dvecs.pop(j)
				dvec_lengths.pop(j)
			else:
				j += 1
		i += 1

	return diversified_query_result[0:DIV_SIZE+1]


readWordList()
print "readWordList()"
readSmartDocuments()
print "readSmartDocuments()"
readQueryResults()
print "readQueryResults()"
openDocFiles()
print "openDocFiles()"


for _lambda_ in LAMBDA_ARRAY:
	diversified_query_results = []
	diversified_query_results.append([])
	for i in range(1,QUERY_NO + 1):
		print "*************STARTED*************" + " => diversifyMaxSum(" + str(i) + "," + str(_lambda_) + ") "  + time.strftime('%X %x')
		diversified_query_result = diversifyMaxSum(i,_lambda_)
		diversified_query_results.append(diversified_query_result)
		print "**************ENDED**************" + " => diversifyMaxSum(" + str(i) + "," + str(_lambda_) + ") "  + time.strftime('%X %x')

	output_file = open("MaxSum_" + str(_lambda_) + ".txt","w")
	for i in range(1,QUERY_NO + 1):
		for j in range(1,len(diversified_query_results[i])):
			doc_score = "%.6f" % diversified_query_results[i][j][2]
			output_file.write(str(i) + "\tQ0\t" + str(diversified_query_results[i][j][0]) + "\t" + str(j) + "\t" + str(doc_score) + "\tfs\n")
	output_file.close()

for _lambda_ in LAMBDA_ARRAY:
	diversified_query_results = []
	diversified_query_results.append([])
	for i in range(1,QUERY_NO + 1):
		print "*************STARTED*************" + " => diversifySy(" + str(i) + "," + str(_lambda_) + ") "  + time.strftime('%X %x')
		diversified_query_result = diversifySy(i,_lambda_)
		diversified_query_results.append(diversified_query_result)
		print "**************ENDED**************" + " => diversifySy(" + str(i) + "," + str(_lambda_) + ") "  + time.strftime('%X %x')

	output_file = open("Sy_" + str(_lambda_) + ".txt","w")
	for i in range(1,QUERY_NO + 1):
		for j in range(1,len(diversified_query_results[i])):
			doc_score = "%.6f" % diversified_query_results[i][j][2]
			output_file.write(str(i) + "\tQ0\t" + str(diversified_query_results[i][j][0]) + "\t" + str(j) + "\t" + str(doc_score) + "\tfs\n")
	output_file.close()

print "Script ended at:   " + time.strftime('%X %x')
