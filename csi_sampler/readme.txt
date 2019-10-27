CSI random sampler'in dogru calisabilmesi icin gerekli dosyalar:
1 - Clusterlara ait dokumanlarin oldugu dosyalar // Binarylerin okunmus halleri.
	Ex: cluster<0-99> // Integer listesi olan dosya, her line clustera ait bir <doc_id> iceriyor.
2 - Script outputu sorted degil. Sort edilmesi lazim
	Ex: sort -n <output_of_this_script<sampled_docs_file_create_path>> > <output_of_this_script<sampled_docs_file_create_path>>_sorted
