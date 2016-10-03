import random

DOC_NO = 50220539
NO_OF_CLUSTERS = 100

cluster_concat_file = open("cluster_concat_sorted","w")
for doc_index in range(1, DOC_NO):
	random_cluster_id = random.randint(0,NO_OF_CLUSTERS - 1)
	cluster_concat_file.write(str(doc_index) + " " + str(random_cluster_id) + "\n")