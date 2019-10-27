import os
import sys
import random

__SAMPLE_PERCENTAGE__ = 0.01
__NO_OF_CLUSTERS__ = 100
CLUSTERS = "/home1/grupef/ecank/data/CLUSTERS/docs/"
ACCESS_COUNTS = "/home1/grupef/ecank/data/CSI/desc_access_counts"
DOC_TO_CLUSTER = "/home1/grupef/ecank/data/doc_to_cluster_map"
RESULTS = "/home1/grupef/ecank/data/CSI/access_csi_docids"

cluster_files = [ os.path.join(CLUSTERS,f) for f in os.listdir(CLUSTERS) if os.path.isfile(os.path.join(CLUSTERS,f)) ]
access_counts_file = open(ACCESS_COUNTS)
doc_to_cluster_file = open(DOC_TO_CLUSTER)
doc_to_cluster = [line.split()[1] for line in doc_to_cluster_file.readlines()]
output_file = open(RESULTS, "w")
doc_counter = [None] * __NO_OF_CLUSTERS__
completed_clusters = 0
sampled_doc_ids = []


for cluster_file in cluster_files:
    cluster_id = int(cluster_file.split("cluster")[1])
    cluster_file_desc = open(cluster_file)
    cluster_file_read = cluster_file_desc.readlines()
    cluster_file_desc.close()

    cluster_length = len(cluster_file_read)
    cluster_sample_length = int(cluster_length*__SAMPLE_PERCENTAGE__)
    doc_counter[cluster_id] = cluster_sample_length
    # print("C#", cluster_id, " s#", doc_counter[cluster_id])

with access_counts_file as fp:
    line = fp.readline()
    while line:
        doc_id = int(line.split()[1])
        cluster_id = int(doc_to_cluster[doc_id-1])
        # print("D#", doc_id, " C#", cluster_id)
        if (doc_counter[cluster_id] > 0):
            sampled_doc_ids.append(doc_id)
            doc_counter[cluster_id] = doc_counter[cluster_id] - 1
            # print("Added, rmng for C:", doc_counter[cluster_id])
            if (doc_counter[cluster_id] <= 0):
                completed_clusters = completed_clusters + 1
                print("C", cluster_id, "/ompleted.")

        line = fp.readline()
        if (completed_clusters == __NO_OF_CLUSTERS__):
            break

    sorted_doc_ids = sorted(sampled_doc_ids)
    print("Total #docs:", len(sorted_doc_ids))
    for doc_id in sorted_doc_ids:
        output_file.write(str(doc_id) + '\n')


access_counts_file.close()
output_file.close()
doc_to_cluster_file.close()
