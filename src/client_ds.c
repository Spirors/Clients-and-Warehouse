#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "client_ds.h"

//Initialize the page table
void initialize_table(){
	int i;
	for(i = 0; i < 16; i++){
		table[i] = NULL;
	}
}

//Free the page table
void free_table(){
	int i;
	for(i = 0; i < 16; i++){
		if(table[i] != NULL){
			free(table[i]);
			table[i] = NULL;
		}
	}
}

//Used in command: dump
void print_table(){
	int i;
	int j;
	for(i = 0; i < 16; i++){
		if(table[i] != NULL){
			for(j = 0; j < 4; j++){
				printf("(%d, %d) -> %d\n", i, j, table[i][j]);
			}
		}
	}
}

//Used in command: infotab
void infotab(){
	int i;
	int j;
	int open = 0;
	char line[10];
	int selected = -1;
	for(i = 0; i < 16; i++){
		if(table[i] != NULL){
			open++;
			printf("Table %d is open\n", i);
		}else{
			printf("Table %d is closed\n", i);
		}
	}
	if(open > 0){
		while(1){
			printf("Enter a open table's number\n");
			fgets(line, 10, stdin);
			line[strcspn(line, "\n")] = '\0';
			if(sscanf(line, "%d", &selected) == 1){
				if(selected > 15 || selected < 0){
					fprintf(stderr, "Number enterd is out of bound\n");
				}else{
					if(table[selected] != NULL){
						for(j = 0; j < 4; j++){
							if(table[selected][j] != -1){
								printf("Table %d, entry %d has remote ID of %d\n", selected, j, table[selected][j]);
							}
						}
						break;
					}else{
						fprintf(stderr, "Selected table is not open, try again\n");
					}
				}
			}else{
				fprintf(stderr, "Invalid input\n");
			}
		}
	}else{
		fprintf(stderr, "No open tables\n");
	}
}

//allocate the table with relevant address given by the database
void alloc_table(int address){
	int i;
	int j;
	for(i = 0; i < 16; i++){
		if(table[i] == NULL){
			table[i] = (int*)malloc(4*sizeof(int));
			for(j = 0; j < 4; j++){
				table[i][j] = -1;
			}
			table[i][0] = address;
			return;
		}else{
			for(j = 0; j < 4; j++){
				if(table[i][j] == -1){
					table[i][j] = address;
					return;
				}
			}
		}
	}
}

//Handling if table is full
int is_table_full(){
	int i;
	int j;
	for(i = 0; i < 16; i++){
		if(table[i] == NULL){
			return -1;
		}else{
			for(j = 0; j < 4; j++){
				if(table[i][j] == -1){
					return -1;
				}
			}
		}
	}
	return 0;
}

//Creating the cache
struct cache_ds* new_cache(){
	struct cache_ds* newCache = (struct cache_ds*)malloc(sizeof(struct cache_ds));
	newCache->remote_id = -1;
	newCache->art_name = NULL;

	return newCache;
}

//Initialize the cache, Note: the cache can hold only 4 item
void initialize_cache(){
	int i;
	for(i = 0; i < 4; i++){
		cache[i] = new_cache();
	}
}

//Free cache data structure
void free_cache(){
	int i;
	for(i = 0; i < 4; i++){
		if(cache[i]->art_name != NULL){
			free(cache[i]->art_name);
			cache[i]->art_name = NULL;
		}
		free(cache[i]);
	}
}

//-1 -> cache miss, 0-3 inclusive -> cache hit
int cache_hit(int address){
	int i;
	for(i = 0; i < 4; i++){
		if(cache[i]->remote_id == address){
			return i;
		}
	}
	return -1;
}

//-1 -> not full, 0 -> full
int cache_full(){
	int i;
	for(i = 0; i < 4; i++){
		if(cache[i]->remote_id == -1){
			return -1;
		}
	}
	return 0;
}

//Eviction rule for cache
void cache_evict(int i){
	cache[i]->remote_id = -1;
	if(cache[i]->art_name != NULL){
		free(cache[i]->art_name);
		cache[i]->art_name = NULL;
	}
}

//Adding to the cache
void cache_add(int r_id, char* a_name){
	int i;
	for(i = 0; i < 4; i++){
		if(cache[i]->remote_id == -1){
			cache[i]->remote_id = r_id;
			if(a_name != NULL){
				cache[i]->art_name = (char*)malloc(256*sizeof(char));
				strcpy(cache[i]->art_name, a_name);
			}
			return;
		}
	}
}

//Cache update rule
void cache_update(int i, char* a_name){
	if(cache[i]->art_name == NULL){
		cache[i]->art_name = (char*)malloc(256*sizeof(char));
		strcpy(cache[i]->art_name, a_name);
	}else{
		strcpy(cache[i]->art_name, a_name);
	}
}

//Printing cache
void print_cache(){
	int i;
	for(i = 0; i < 4; i++){
		if(cache[i]->remote_id != -1){
			if(cache[i]->art_name == NULL){
				printf("(%d) -> %d, N/A\n", i, cache[i]->remote_id);
			}else{
				printf("(%d) -> %d, %s\n", i, cache[i]->remote_id, cache[i]->art_name);
			}
		}else{
			printf("(%d) -> -1\n", i);
		}
	}
}
