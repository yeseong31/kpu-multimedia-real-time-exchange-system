#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <stdbool.h>
#include <string.h>
#include <fcntl.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <pthread.h>
#include <unistd.h>

#define KEY_NUM 60069
#define MAX_FILE_NAME 100
#define MAX_FILE_PATH 100
#define NUM_OF_SHM 2
#define MEM_SIZE 2048 

struct sh_use_st{
	int shm_id;
	int wr_flag;
	int end_flag;
	int read_cnt;
	char sendFileName[MAX_FILE_NAME];
	char recvFileName[MAX_FILE_NAME];
	char msg[MEM_SIZE];
	void * shm_addr;
	pthread_mutex_t mutex;
};

void init_struct(struct sh_use_st *);
struct sh_use_st * make_shm(int);
int destroy_shm(struct sh_use_st *);
int send_file(struct sh_use_st *);
int recv_file(struct sh_use_st *);
void create_thread(void *, struct sh_use_st *);
void send_thread(struct sh_use_st *);
void recv_thread(struct sh_use_st *);
