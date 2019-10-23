#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <ctype.h>
#include <string.h>

#include <pthread.h>

#include <signal.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "client_ds.h"

#define BUFFER 300
#define CMD_BUF 7
#define NAME_BUF 255

#define PIPE_BUF 300

void lowercase(char* str){
	for(int i = 0; str[i]; i++){
		str[i] = tolower(str[i]);
	}
}

void int_handler(int n){
	sigset_t x;
	sigemptyset(&x);
	sigaddset(&x, SIGINT);
	sigprocmask(SIG_BLOCK, &x, NULL);
}

//Method for sending and receiving through pipes
int send_msg(char* msg, char* filename);
int receive_msg(char* buf, char* filename);

int start_flag = 0;
int server_down = 0;

//The start thread
void* thread(void* arg);

int main(int argc, char* argv[]){
	char* pipe;

	if(argc != 2){
		fprintf(stderr, "wrong number of arguments\n");
		return -1;
	}

	pipe = argv[1];

	struct stat sb;
	if(stat(pipe, &sb) == -1){
		fprintf(stderr, "File does not exist: %s\n", pipe);
		return -1;
	}

	signal(SIGINT, int_handler);
	pthread_t tid;

	char line[BUFFER];
	char cmd[CMD_BUF];
	int id;
	char Z[NAME_BUF];

	//For communication
	char msg[PIPE_BUF];
	char buf[PIPE_BUF];


	//The shell that handles all the commands
	while(1){
		printf("shell> ");
		fgets(line, BUFFER, stdin);
		line[strcspn(line, "\n")] = '\0';
		lowercase(line);

		if(strcmp(line, "exit") == 0){
			//If close is not call and a session is still going
			if(start_flag == 1){
				sprintf(msg, "close %ld\n", tid);
				if(send_msg(msg, pipe) != 0){
					server_down = 1;
					break;
				}

				if(receive_msg(buf, pipe) != 0){
					server_down = 1;
					break;
				}
				buf[strcspn(buf, "\n")] = '\0';
				printf("%s\n", buf);

				free_table();
				free_cache();
				start_flag = 0;
				pthread_join(tid, NULL);
			}
			break;
		}else if(strcmp(line, "start") == 0){
			if(start_flag == 0){
				sprintf(msg, "start %ld\n", tid);
				if(send_msg(msg, pipe) != 0){
					server_down = 1;
					break;
				}

				if(receive_msg(buf, pipe) != 0){
					server_down = 1;
					break;
				}
				buf[strcspn(buf, "\n")] = '\0';
				printf("%s\n", buf);

				start_flag = 1;
				pthread_create(&tid, NULL, thread, NULL);

				initialize_table();
				initialize_cache();
			}else{
				fprintf(stderr, "A session is currently running\n");
			}
		}else if(strcmp(line, "close") == 0){
			if(start_flag == 1){
				sprintf(msg, "close %ld\n", tid);
				if(send_msg(msg, pipe) != 0){
					server_down = 1;
					break;
				}

				if(receive_msg(buf, pipe) != 0){
					server_down = 1;
					break;
				}
				buf[strcspn(buf, "\n")] = '\0';
				printf("%s\n", buf);

				free_table();
				free_cache();
				start_flag = 0;
				pthread_join(tid, NULL);
			}else{
				fprintf(stderr, "There is no session running\n");
			}
		}else if(strcmp(line, "alloc") == 0){
			if(start_flag == 1){
				if(is_table_full() != 0){
					sprintf(msg, "alloc %ld\n", tid);
					if(send_msg(msg, pipe) != 0){
						server_down = 1;
						break;
					}

					if(receive_msg(buf, pipe) != 0){
						server_down = 1;
						break;
					}
					buf[strcspn(buf, "\n")] = '\0';
					int address = atoi(buf);
					if(address < 0){
						printf("Failure database is full!\n");
					}else{
						alloc_table(address);
						printf("Success!\n");
					}
				}else{
					fprintf(stderr, "Table is full\n");
				}
			}else{
				fprintf(stderr, "There is no session running\n");
			}
		}else if(strcmp(line, "dump") == 0){ //Used for bug testing
			if(start_flag == 1){
				printf("-------Table-------\n");
				print_table();
				printf("-------Cache-------\n");
				print_cache();
			}else{
				fprintf(stderr, "There is no session running\n");
			}
		}else if(strcmp(line, "infotab") == 0){
			if(start_flag == 1){
				infotab();
			}else{
				fprintf(stderr, "There is no session running\n");
			}
		}else if(strncmp(line, "dealloc", 7) == 0){
			if(sscanf(line, "%s %d", cmd, &id) == 2){
				if(id < 0 || id > 63){
					fprintf(stderr, "Please enter a valid id, 0->63 inclusive\n");
				}else{
					if(start_flag == 1){
						int inner_index = id & 3;
						int outer_index = (id & 60)>>2;

						if(table[outer_index] == NULL){
							fprintf(stderr, "Nothing in the table\n");
						}else if(table[outer_index][inner_index] == -1){
							fprintf(stderr, "Nothing in the table\n");
						}else{
							int address = table[outer_index][inner_index];

							sprintf(msg, "dealloc %ld %d\n", tid, address);
							if(send_msg(msg, pipe) != 0){
								server_down = 1;
								break;
							}

							if(receive_msg(buf, pipe) != 0){
								server_down = 1;
								break;
							}
							buf[strcspn(buf, "\n")] = '\0';
							printf("%s\n", buf);

							int cache_num = cache_hit(address);
							if(cache_num != -1){
								printf("cache hit\n");
								cache_evict(cache_num);
								printf("eviction\n");
							}else{
								printf("cache miss\n");
							}

							table[outer_index][inner_index] = -1;
						}
					}else{
						fprintf(stderr, "There is no session running\n");
					}
				}
			}else{
				fprintf(stderr, "wrong input\n");
			}
		}else if(strncmp(line, "read", 4) == 0){
			if(sscanf(line, "%s %d", cmd, &id) == 2){
				if(id < 0 || id > 63){
					fprintf(stderr, "Please enter a valid id, 0->63 inclusive\n");
				}else{
					if(start_flag == 1){
						int inner_index = id & 3;
						int outer_index = (id & 60)>>2;

						if(table[outer_index] == NULL){
							fprintf(stderr, "Nothing in the table\n");
						}else if(table[outer_index][inner_index] == -1){
							fprintf(stderr, "Nothing in the table\n");
						}else{
							int address = table[outer_index][inner_index];

							int cache_num = cache_hit(address);
							if(cache_num != -1){
								printf("cache hit\n");
								if(cache[cache_num]->art_name == NULL){
									printf("N/A, please store a name first\n");
								}else{
									printf("%s\n", cache[cache_num]->art_name);
								}
							}else{
								printf("cache miss\n");
								sprintf(msg, "read %ld %d\n", tid, address);
								if(send_msg(msg, pipe) != 0){
									server_down = 1;
									break;
								}

								if(receive_msg(buf, pipe) != 0){
									server_down = 1;
									break;
								}
								buf[strcspn(buf, "\n")] = '\0';
								if(strcmp(buf, "N/A") == 0){
									printf("%s, please store a name first\n", buf);
									if(cache_full() == 0){
										cache_evict(3);
										printf("eviction\n");
										cache_add(address, NULL);
									}else{
										cache_add(address, NULL);
									}
								}else{
									printf("%s\n", buf);
									if(cache_full() == 0){
										cache_evict(3);
										printf("eviction\n");
										cache_add(address, buf);
									}else{
										cache_add(address, buf);
									}
								}
							}
						}
					}else{
						fprintf(stderr, "There is no session running\n");
					}
				}
			}else{
				fprintf(stderr, "wrong input\n");
			}
		}else if(strncmp(line, "store", 5) == 0){
			if(sscanf(line, "%s %d %[^\" ]", cmd, &id, Z) == 3){
				if(id < 0 || id > 63){
					fprintf(stderr, "Please enter a valid id, 0->63 inclusive\n");
				}else{
					if(start_flag == 1){
						int inner_index = id & 3;
						int outer_index = (id & 60)>>2;

						if(table[outer_index] == NULL){
							fprintf(stderr, "Nothing in the table\n");
						}else if(table[outer_index][inner_index] == -1){
							fprintf(stderr, "Nothing in the table\n");
						}else{
							int address = table[outer_index][inner_index];

							sprintf(msg, "store %ld %d %s\n", tid, address, Z);
							if(send_msg(msg, pipe) != 0){
								server_down = 1;
								break;
							}

							if(receive_msg(buf, pipe) != 0){
								server_down = 1;
								break;
							}
							buf[strcspn(buf, "\n")] = '\0';
							printf("%s\n", buf);

							int cache_num = cache_hit(address);
							if(cache_num != -1){
								printf("cache hit\n");
								cache_update(cache_num, Z);
							}else{
								printf("cache miss\n");
								if(cache_full() == 0){
									cache_evict(3);
									printf("eviction\n");
									cache_add(address, Z);
								}else{
									cache_add(address, Z);
								}
							}
						}
					}else{
						fprintf(stderr, "There is no session running\n");
					}
				}
			}else if(sscanf(line, "%s %d \"%[^\"]\"", cmd, &id, Z) == 3){
				if(id < 0 || id > 63){
					fprintf(stderr, "Please enter a valid id, 0->63 inclusive\n");
				}else{
					if(start_flag == 1){
						int inner_index = id & 3;
						int outer_index = (id & 60)>>2;

						if(table[outer_index] == NULL){
							fprintf(stderr, "Nothing in the table\n");
						}else if(table[outer_index][inner_index] == -1){
							fprintf(stderr, "Nothing in the table\n");
						}else{
							int address = table[outer_index][inner_index];

							sprintf(msg, "store %ld %d ", tid, address);
							strcat(msg, "\"");
							strcat(msg, Z);
							strcat(msg, "\"\n");

							if(send_msg(msg, pipe) != 0){
								server_down = 1;
								break;
							}

							if(receive_msg(buf, pipe) != 0){
								server_down = 1;
								break;
							}
							buf[strcspn(buf, "\n")] = '\0';
							printf("%s\n", buf);

							int cache_num = cache_hit(address);
							if(cache_num != -1){
								printf("cache hit\n");
								cache_update(cache_num, Z);
							}else{
								printf("cache miss\n");
								if(cache_full() == 0){
									cache_evict(3);
									printf("eviction\n");
									cache_add(address, Z);
								}else{
									cache_add(address, Z);
								}
							}
						}
					}else{
						fprintf(stderr, "There is no session running\n");
					}
				}
			}else{
				fprintf(stderr, "wrong inputs\n");
			}
		}else{
			fprintf(stderr, "wrong command\n");
		}
	}
	if(server_down == 1){
		fprintf(stderr, "Database Shutdown\n");
		if(start_flag == 1){
			free_table();
			free_cache();
			start_flag = 0;
			pthread_join(tid, NULL);
		}
		return -1;
	}
	return 0;
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

//The thread of this client
void* thread(void* arg){
	while(start_flag);
	return NULL;
}
