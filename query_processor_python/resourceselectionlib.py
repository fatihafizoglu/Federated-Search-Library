class Redde:

	def __init__(self, heap, document_cluster_map):

		cluster_score_dict = {}
		for (doc_score,doc_id) in heap:
			cluster_id = document_cluster_map[doc_id]
			try:
				cluster_score_dict[cluster_id] += 1
			except KeyError:
				cluster_score_dict[cluster_id] = 1

		self.sorted_clusters = sorted(cluster_score_dict.items(),key = lambda x:x[1])
		self.sorted_clusters.reverse()

	def get_top_k_clusters(self,k):

		return [x[0] for x in self.sorted_clusters[0:k]]