import os
import time
import math

wordlist_file_path = "/home/eckucukoglu/projects/ms-thesis/allocation_runs/topic_based_1/csi_wordlist.txt"
new_wordlist_file_path = "/home/eckucukoglu/projects/ms-thesis/allocation_runs/topic_based_1/csi_wordlist_idf.txt"
new_wordlist_file = open(new_wordlist_file_path,"w")

# DOC_NUM = 50220538 # ALL
DOC_NUM = 502252 # TopicBasedClusters_100
# DOC_NUM = 502254 # TopicBasedClusters_100_2


with open(wordlist_file_path) as f:
		for line in f:
			line_splitted = line.split(" ")

			if int(line_splitted[1]) != 0:
				idf = math.log(1+(DOC_NUM/int(line_splitted[1])),2)
			else:
				idf = -1

			# print "{} {} {}".format(line_splitted[0],int(line_splitted[1]),idf)
			new_wordlist_file.write("{} {} {}\n".format(line_splitted[0],int(line_splitted[1]),idf))
new_wordlist_file.close()
