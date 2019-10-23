Client Database Project.

This a extra credit project for my system class.
Requirement:
	use two page table in clients.
	use cache in clients.
	use either FIFO pipes or network programming to perform communication.
	create commands that demonstrate this project.

In this project I was instructed to create a client and warehouse
emulating clients and a database of art entry.

I decided to follow the suggestion of emulating the communication
between the clients and the database through FIFO pipes. (other way being network programming)

The commands of each file was designed similarly to the homework assignment of this class.

Headers:

	inc/client_ds.h ->	contains data structures used in
						src/client.c and src/client_ds.c

	inc/warehouse_ds.h ->	contains data structures used in
							src/warehouse_db.c and src/warehouse_ds.c


Main programs:

	CAUTION: Please use the correct pipe and do not open the same pipe using different clients
	NOTE: This project should be run in multiple window to fully visualize the communication between clients and the database.

	make -> client and warehouse_db

	./warehouse_db [size of database]
		**Create four pipe: fifo_client1, fifo_client2, fifo_client3, fifo_client4**

		-Four thread each thread handling an pipe which is hard wired for this project.

		-Data structure:	an size n array of struct art_entry* which contains art name, valid bit, and client id

		-Handle SIGINT by redirecting user into an shell with following commands:
			```
			list -> list out all connected client's thread id
			list [client id] -> list out all art entry with the same client id
			dump -> list out everything in the database
			exit -> exit out of the program and unlink the four pipes
			```

	./client [pipe name]
		-Contains a two level page table
		-Contains a cache of size 4

		-Data structure:	an int** table with lazy allocation to mimick two page table
							an size 4 array of struct cache_ds* which contains remote id and art name mimicking the cache

		-Contains command:
			```
			start -> start the thread and connect to database if possible
			close -> end the thread and disconnect from database if needed. Free everything
			exit -> end the thread and disconnect from database if needed. Free everything
			alloc -> allocate a space in database and insert the remote id to the table
			dealloc [local id] -> deallocate the space in database and remove remote id from the table. Also uses cache. (explained later)
			read [local id] -> read the name of an remote id in the table using local id to translate through it. Also uses cache. (explained later)
			store [local id] [name] -> store the name into the art entry with the remote id translated from local id. Also uses cache. (explained later)
			infotab -> interactive cmd used to navagate the table
			dump -> added command that print out everything in the table and cache. Mainly used in process of debugging.
			```
			ps: local id is 0 - 63 inclusive. As the max number of remote id a table can handle is 64

		Note: ./client [pipe name] need to ran after ./warehouse_db [size].
			  ./client will terminate if ./warehouse_db exit before client exit. printing "Server Shutdown"


Cache:

	Cache is a array of size 4 containing remote id and art name

	read ->		will first translate the table and read the remote id and if the cache has that remote id "cache hit" is printed and the name will be read.
				if "cache miss" is printed then the client communicate to the server for the name and the store that remote id and name into the cache
					if cache is full then evict at the last cache element which is cache[3]

	dealloc ->	the communication to server happens no matter what. After the communication if the "cache hit" the dealloced remote id, that cache will be evicted.

	store ->	the communication to the server happens no matter what. After the communication if the "cache hit" then that cache will be updated.
				if "cache miss" then that cache will be added to the cache following the same eviction procedure as the read command.

	cache will be free if needed in close and exit command.


Error handling:

	when entering wrong command there will be error msg printed out
	when an action fails such as alloc when database is full, a error msg will be printed
