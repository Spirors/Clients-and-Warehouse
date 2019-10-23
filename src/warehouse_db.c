#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <ctype.h>
#include <string.h>

#include <semaphore.h>
#include <pthread.h>

#include <signal.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "warehouse_ds.h"

#define BUFFER 300
#define CMD_BUF 4

#define PIPE_BUF 300

void lowercase(char* str){
	for(int i = 0; str[i]; i++){
		str[i] = tolower(str[i]);
	}
}

int send_msg(char* msg, char* filename){
	int fd;

	if((fd = open(filename, O_WRONLY)) < 0){
		return -1;
	}

	int a = write(fd, msg, strlen(msg)+1);
	close(fd);
	return 0;
}

int receive_msg(char* buf, char* filename){
	int fd;

	if((fd = open(filename, O_RDONLY)) < 0){
		return -1;
	}

	int a = read(fd, buf, PIPE_BUF);
	close(fd);
	return 0;
}

//For printing colors
void pick_color(int color_num){
	if(color_num == 0){
		printf("\033[1;31m"); //Red
	}else if(color_num == 1){
		printf("\033[1;33m"); //Yellow
	}else if(color_num == 2){
		printf("\033[0;32m"); //Green
	}else if(color_num == 3){
		printf("\033[0;34m"); //Blue
	}else{
		printf("\033[0m"); //Reset
	}
}
void* thread(void* arg);
int alloc(pthread_t c_id);

//Reason why add/remove client was done here is because mutex needed.
int add_client(pthread_t c_id);
int remove_client(pthread_t c_id);
sem_t mutex, w;

char* pipe1 = "fifo_client1";
char* pipe2 = "fifo_client2";
char* pipe3 = "fifo_client3";
char* pipe4 = "fifo_client4";

pthread_t tid1, tid2, tid3, tid4;
//Handler for SIGINT
void handler(int n){

	pick_color(-1);

	char line[BUFFER];
	char cmd[CMD_BUF];
	pthread_t c_id;

	printf("shell> ");

	fgets(line, BUFFER, stdin);
	line[strcspn(line, "\n")] = '\0';
	lowercase(line);

	if(strcmp(line, "exit") == 0){
		free_db();
		unlink(pipe1);
		unlink(pipe2);
		unlink(pipe3);
		unlink(pipe4);
		exit(0);
	}else if(strcmp(line, "list") == 0){
		list();
	}else if(strcmp(line, "dump") == 0){
		dump();
	}else if(strncmp(line, "list", 4) == 0){
		if(sscanf(line, "%s %ld", cmd, &c_id) == 2){
			list_id(c_id);
		}else{
			fprintf(stderr, "wrong input\n");
		}
	}else{
		fprintf(stderr, "wrong command\n");
	}
	signal(SIGINT, handler);
}

int main(int argc, char* argv[]){

	pick_color(-1);

	if(argc != 2){
		fprintf(stderr, "wrong number of arguments\n");
		return -1;
	}

	size = atoi(argv[1]);
	if(size < 0){
		fprintf(stderr, "number of records should be positive\n");
		return -1;
	}

	initialize_db();

	signal(SIGINT, handler);

	sem_init(&mutex, 0, 1);
	sem_init(&w, 0, 1);

	int client1, client2, client3, client4;

	client1 = mkfifo(pipe1,0666);
	if(client1 < 0) {
		fprintf(stderr, "unable to create a fifo\n");
		return -1;
	}

	client2 = mkfifo(pipe2,0666);
	if(client2 < 0) {
		fprintf(stderr, "unable to create a fifo\n");
		return -1;
	}

	client3 = mkfifo(pipe3,0666);
	if(client3 < 0) {
		fprintf(stderr, "unable to create a fifo\n");
		return -1;
	}

	client4 = mkfifo(pipe4,0666);
	if(client4 < 0) {
		fprintf(stderr, "unable to create a fifo\n");
		return -1;
	}
	printf("four fifo client have been successfully created\n");

	pthread_create(&tid1, NULL, thread, (void*)pipe1);
	pthread_create(&tid2, NULL, thread, (void*)pipe2);
	pthread_create(&tid3, NULL, thread, (void*)pipe3);
	pthread_create(&tid4, NULL, thread, (void*)pipe4);

	while(1);

	return 0;
}

int alloc(pthread_t c_id){
	sem_wait(&mutex);
	int retval = alloc_art_entry(c_id);
	sem_post(&mutex);
	return retval;
}

int add_client(pthread_t c_id){
	sem_wait(&w);
	int i;
	for(i = 0; i < 4; i++){
		if(client_ids[i] == 0){
			client_ids[i] = c_id;
			break;
		}
	}
	sem_post(&w);
	return i;
}

int remove_client(pthread_t c_id){
	sem_wait(&w);
	int i;
	for(i = 0; i < 4; i++){
		if(client_ids[i] == c_id){
			client_ids[i] = 0;
			break;
		}
	}
	sem_post(&w);
	return i;
}


void* thread(void* arg){
	pthread_detach(pthread_self());
	char* pipe = (char*)arg;
	char buf[PIPE_BUF];
	char msg[PIPE_BUF];

	pthread_t client_id;
	char action[10];
	int index;
	char name[255];

	int color_num;

	//The shell for database
	while(1){
		if(receive_msg(buf, pipe) != 0){
			return NULL;
		}
		sleep(1);

		buf[strcspn(buf, "\n")] = '\0';
		if(strncmp(buf, "start", 5) == 0){
			sscanf(buf, "%s %ld", action, &client_id);

			color_num = add_client(client_id);
			pick_color(color_num);

			printf("Client: %ld, connected\n", client_id);
			sprintf(msg, "Connected!\n");
		}else if(strncmp(buf, "close", 5) == 0){
			sscanf(buf, "%s %ld", action, &client_id);
			remove_client(client_id);
			free_id(client_id);

			pick_color(color_num);

			printf("Client: %ld, disconnected\n", client_id);
			sprintf(msg, "Disconnected!\n");
		}else if(strncmp(buf, "alloc", 5) == 0){
			sscanf(buf, "%s %ld", action, &client_id);
			int retval = alloc(client_id);

			pick_color(color_num);

			if(retval < 0){
				printf("Client: %ld, alloc failure\n", client_id);
			}else{
				printf("Client: %ld, alloc at record %d\n", client_id, retval);
			}
			sprintf(msg, "%d\n", retval);
		}else if(strncmp(buf, "dealloc", 7) == 0){
			sscanf(buf, "%s %ld %d", action, &client_id, &index);
			dealloc_art_entry(index);

			pick_color(color_num);

			printf("Client: %ld, dealloc at record %d\n", client_id, index);
			sprintf(msg, "Dealloc completed\n");
		}else if(strncmp(buf, "read", 4) == 0){
			sscanf(buf, "%s %ld %d", action, &client_id, &index);
			char* art_name = NULL;
			art_name = read_art_entry(index);

			pick_color(color_num);

			printf("Client: %ld, read at record %d\n", client_id, index);
			sprintf(msg, "%s\n", art_name);
		}else if(strncmp(buf, "store", 5) == 0){
			if(sscanf(buf, "%s %ld %d %[^\" ]", action, &client_id, &index, name) == 4){
				store_art_entry(index, name);

				pick_color(color_num);

				printf("Client: %ld, store %s at record %d\n", client_id, name, index);
				sprintf(msg, "Store completed\n");
			}else if(sscanf(buf, "%s %ld %d \"%[^\"]\"", action, &client_id, &index, name) == 4){
				store_art_entry(index, name);

				pick_color(color_num);

				printf("Client: %ld, store %s at record %d\n", client_id, name, index);
				sprintf(msg, "Store completed\n");
			}
		}

		if(send_msg(msg, pipe) != 0){
			return NULL;
		}
	}
	return NULL;
}
