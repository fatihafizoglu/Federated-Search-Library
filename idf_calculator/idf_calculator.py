import os
import time
import math

# DOC_NUM = 50220538 # ALL
# DOC_NUM = 502252 # TopicBasedClusters_100
# DOC_NUM = 502254 # TopicBasedClusters_100_2

DOC_NUMS_FOR_TOPIC_CLUSTERS_100_2=[ 442945, 1148464, 295856, 1479512, 666299, 212940, 135474, 61678, 156910, 51775,
           696675, 164655, 283057, 187758, 361508, 616382, 76461, 20039, 1108236, 146654,
           461727, 39677, 416165, 1049260, 24453, 236924, 383599, 13869, 313699, 96936,
           103481, 936669, 517065, 96911, 324628, 247542, 435022, 39311, 280209, 316679,
           28993, 1505640, 613453, 408728, 493420, 325072, 535009, 473867, 456854, 319559,
           287647, 150230, 705544, 214754, 951097, 3425323, 44696, 482774, 740198, 225262,
           127898, 252595, 277183, 306843, 851317, 557803, 425404, 45005, 143081, 152998,
           295321, 196283, 108363, 530840, 88748, 121641, 56635, 139588, 1852188, 52457,
           912724, 362132, 162742, 1398808, 256123, 623919, 311424, 19190, 35281, 414926,
           2402024, 883552, 612048, 1637268, 13895, 1437136, 4493721, 211258, 42849, 372103 ]

wordlist_file_paths_for_topic_clusters_100_2="/home1/grupef/ecank/data/CLUSTERS/wordlist_"

for x in range(100):
    wordlist_file_path = wordlist_file_paths_for_topic_clusters_100_2 + str(x)
    new_wordlist_file_path = wordlist_file_path + "_IDF"
    new_wordlist_file = open(new_wordlist_file_path,"w")
    DOC_NUM = DOC_NUMS_FOR_TOPIC_CLUSTERS_100_2[x]
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
