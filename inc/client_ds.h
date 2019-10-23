#ifndef CLIENT_DS_H
#define CLIENT_DS_H

//Two level page table
int* table[16];

//Functions with documentation in client_ds.c
void initialize_table();
void free_table();
void print_table();
void infotab();
void alloc_table(int address);
int is_table_full();

//Cache data struct
struct cache_ds{
	int remote_id;
	char* art_name;
};

struct cache_ds* cache[4];

//Functions with documentation in client_ds.c
struct cache_ds* new_cache();
void initialize_cache();
void free_cache();
int cache_hit(int address);
int cache_full();
void cache_evict();
void cache_add(int r_id, char* a_name);
void cache_update(int i, char* a_name);
void print_cache();

#endif /* CLIENT_DS_H */
