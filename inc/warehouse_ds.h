#ifndef WAREHOUSE_DS_H
#define WAREHOUSE_DS_H

#include <pthread.h>

//Data struct for art entry
struct art_entry{
	char* art_name;
	pthread_t client_id;
	int bit;
};

//Hard coded threads to handle the MAX of four clients
pthread_t client_ids[4];

//The database structure
struct art_entry** warehouse;

//Size parameter to receive the size given by the user
int size;

//Functions with documentation in warehouse_ds.c
struct art_entry* new_art();
void initialize_db();
void free_db();
void list();
void list_id(pthread_t c_id);
void free_id(pthread_t c_id);
void dump();
int alloc_art_entry(pthread_t c_id);
void dealloc_art_entry(int index);
char* read_art_entry(int index);
void store_art_entry(int index, char* name);

#endif /* WAREHOUSE_DS_H */
