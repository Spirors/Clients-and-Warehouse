#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "warehouse_ds.h"

//used to create new art entry
struct art_entry* new_art(){
	struct art_entry* new_art = (struct art_entry*)malloc(sizeof(struct art_entry));
	new_art->art_name = NULL;
	new_art->client_id = -1;
	new_art->bit = 0;
	return new_art;
}

//initialized the database
void initialize_db(){
	warehouse = (struct art_entry**)malloc(size*sizeof(struct art_entry*));
	int i;
	for(i = 0; i < size; i++){
		warehouse[i] = new_art();
	}
}

//Null check not require for warehouse or warehouse[i] as initialize_db() is called in main()
void free_db(){
	int i;
	for(i = 0; i < size; i++){
		if(warehouse[i]->art_name != NULL){
			free(warehouse[i]->art_name);
			warehouse[i]->art_name = NULL;
		}
		free(warehouse[i]);
	}
	free(warehouse);
}

//Used in command: list
void list(){
	int i;
	for(i = 0; i < 4; i++){
		if(client_ids[i] != 0){
			printf("Client: %ld\n", client_ids[i]);
		}
	}
}

//Used in command: list [id]
void list_id(pthread_t c_id){
	int i;
	for(i = 0; i < size; i++){
		if(warehouse[i]->client_id == c_id){
			if(warehouse[i]->art_name == NULL){
				printf("RecordID: %d, ClientID: %ld, Name: N/A, Bit: %d\n", i, warehouse[i]->client_id, warehouse[i]->bit);
			}else{
				printf("RecordID: %d, ClientID: %ld, Name: %s, Bit: %d\n", i, warehouse[i]->client_id, warehouse[i]->art_name, warehouse[i]->bit);
			}
		}
	}
}

//Free the art entry with id: c_id
void free_id(pthread_t c_id){
	int i;
	for(i = 0; i < size; i++){
		if(warehouse[i]->client_id == c_id){
			if(warehouse[i]->art_name != NULL){
				free(warehouse[i]->art_name);
				warehouse[i]->art_name = NULL;
				warehouse[i]->client_id = -1;
				warehouse[i]->bit = 0;
			}else{
				warehouse[i]->client_id = -1;
				warehouse[i]->bit = 0;
			}
		}
	}
}

//Used in command: dump
void dump(){
	int i;
	for(i = 0; i < size; i++){
		if(warehouse[i]->art_name == NULL){
			printf("RecordID: %d, ClientID: %ld, Name: N/A, Bit: %d\n", i, warehouse[i]->client_id, warehouse[i]->bit);
		}else{
			printf("RecordID: %d, ClientID: %ld, Name: %s, Bit: %d\n", i, warehouse[i]->client_id, warehouse[i]->art_name, warehouse[i]->bit);
		}
	}
}

//Allocate art entry for respective id
int alloc_art_entry(pthread_t c_id){
	int i;
	for(i = 0; i < size; i++){
		if(warehouse[i]->bit == 0){
			warehouse[i]->bit = 1;
			warehouse[i]->client_id = c_id;
			return i;
		}
	}
	return -1;
}

//Dealloc an art entry
void dealloc_art_entry(int index){
	warehouse[index]->bit = 0;
	warehouse[index]->client_id = -1;
	if(warehouse[index]->art_name != NULL){
		free(warehouse[index]->art_name);
		warehouse[index]->art_name = NULL;
	}
}

//For reading the name of an art entry
char* read_art_entry(int index){
	char* none = "N/A";
	if(warehouse[index]->art_name == NULL){
		return none;
	}else{
		return warehouse[index]->art_name;
	}
}

//For storing the name of an art entry
void store_art_entry(int index, char* name){
	if(warehouse[index]->art_name == NULL){
		warehouse[index]->art_name = (char*)malloc(256*sizeof(char));
		strcpy(warehouse[index]->art_name, name);
	}else{
		strcpy(warehouse[index]->art_name, name);
	}
}
