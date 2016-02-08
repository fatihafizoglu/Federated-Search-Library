Index splitter'in dogru calisabilmesi icin gerekli dosyalar:
1 - Index dosyasi
	Ex: merged_entry.txt // Binary dosya, <doc_id,occurance_count> pairleri
2 - WordList dosyasi
	Ex: merged_wordlist.txt
	Format: <term,occurance_count,idf(Nasil hesaplandigini henuz bilmiyoruz)
		00 551509 91.060068
		000 894703 56.130897
		0000 139597 359.751128
		00000 24653 2037.013791
		000000 374955 133.937150
3 - Doc_ID - Cluster_ID mapi
	Ex: cluster_concat_sorted
	Format: <document_id,cluster_id>
		1 80
		2 10
		3 55
		4 55
		5 41
		...
		...
		50220534 96
		50220535 96
		50220536 96
		50220537 58
		50220538 96
	Notes:
		Binary document_id lerin bulundugu dosyalardan bu mapi nasil cikarabiliriz?
		Binary document_id'lerin bulundugu dizin _PATH_ olsun
		1.step:
			>>	mkdir _PATH_/read
			>>	for i in {0..NO_OF_CLUSTER/*99*/}; do
					./read_clusters _PATH_/cluster$i > _PATH_/read/cluster$i
				done
			Summary: Binary dosyalar okunur, her satirda bir document_id olacak sekilde cluster'a "read" dizini icerisinde yeni bir dosya olusturulur.
		2.step:
			>>	mkdir _PATH_/read/final
			>>	for i in {0..NO_OF_CLUSTER/*99*/}; do
					while read line; do
						echo "$line $i" >> _PATH_/read/final/cluster_concat
					done
				done
			Summary: 1.stepte olusturulan tum dosyalar sirayla okunur ve dosya icerisindeki line'lar baska bir dosyaya aynen yazilir. Line'in sonuna ekstradan bir bosluk ve cluster_id si yazilir.
		3.step:
			>> sort -n _PATH_/read/final/cluster_concat > _PATH_/read/final/cluster_concat_sorted
			Summary: 2.stepte olustulan dosya document idlere gore sort edilir.
