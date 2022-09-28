#include "header.h"

//공유메모리 구조체 초기화 함수
void init_struct(struct sh_use_st * shmsg){
	shmsg->shm_id = 0;
	shmsg->wr_flag = 0;
	shmsg->end_flag = 0;
	shmsg->read_cnt = 0;

	shmsg->shm_addr = NULL;

	pthread_mutex_init(&shmsg->mutex, NULL);
}

//공유메모리 생성 함수
struct sh_use_st * make_shm(int key_num){
	int shm_id;
	void * shm_addr;

	if ((shm_id = shmget((key_t)key_num, MEM_SIZE, IPC_CREAT|0666)) == -1){
		printf("Shmget failed\n");
		exit(EXIT_FAILURE);
	}
	if ((shm_addr = shmat(shm_id, (void *)0, 0)) == (void*)-1){
		printf("Shmat failed\n");
		exit(EXIT_FAILURE);
	}
	struct sh_use_st *shmsg = (struct sh_use_st *) shm_addr;
	//init_struct(shmsg);
	shmsg->shm_addr = shm_addr;
	shmsg->shm_id = shm_id;
	return shmsg;
}

//공유메모리 제거 함수
int destroy_shm(struct sh_use_st * shmsg)
{
	int id = shmsg->shm_id;
	void * addr = shmsg->shm_addr;

	if (shmdt(addr) == -1){
		printf("shmdt failed\n");
		return -1;
	}
	if (shmctl(id, IPC_RMID, 0) == -1){
		printf("shmctl failed\n");
		return -1;
	}
	return 1;
}

//파일 전송을 위한 함수
int send_file(struct sh_use_st * shmsg){
	FILE *fp;

	if(!(fp = fopen(shmsg->sendFileName, "rb"))){
		printf("%s file open error\n", shmsg->sendFileName);
		return -1;
	}

	while(1){
		if(shmsg->wr_flag == 0){
			shmsg->read_cnt = fread(shmsg->msg, 1, MEM_SIZE, fp);
			//if(shmsg->read_cnt<MEM_SIZE){
			if(feof(fp)){
				shmsg->end_flag = 1;
				printf("file send completed\n");
				break;
			}
			shmsg->wr_flag = 1;
		}
	}
	fclose(fp);
	return 1;
}

//파일 수신을 위한 함수
int recv_file(struct sh_use_st * shmsg){
	FILE *fp;

	if(!(fp = fopen(shmsg->recvFileName, "wb"))){
		printf("%s file open error\n", shmsg->recvFileName);
		return -1;
	}

	while(1){
		if(shmsg->wr_flag == 1){
			fwrite(shmsg->msg, 1, shmsg->read_cnt, fp);
			shmsg->wr_flag = 0;
		}
		else if(shmsg->end_flag == 1){
			fwrite(shmsg->msg, 1, shmsg->read_cnt, fp);
			printf("file recv completed\n");
			shmsg->wr_flag = 0;
			shmsg->end_flag = 0;
			shmsg->read_cnt = 0;
			break;
		}
	}
	fclose(fp);
	return 1;
}

//쓰레드 생성 함수
void create_thread(void * func, struct sh_use_st * arg)
{
	pthread_t thread;
	int tid;
	if ((tid = pthread_create(&thread, NULL, func, (void*)arg)) < 0){
		printf("ERROR! pthread create error");
		exit(1);
	}
	pthread_join(thread, NULL);
}

//쓰레드를 이용한 파일 전송 함수
void send_thread(struct sh_use_st * arg){
	pthread_mutex_lock(&arg->mutex);
	send_file(arg);
	pthread_mutex_unlock(&arg->mutex);
	pthread_exit((void*)NULL);
}

//쓰레드를 이용한 파일 수신 함수
void recv_thread(struct sh_use_st * arg){
	pthread_mutex_lock(&arg->mutex);
	recv_file(arg);
	pthread_mutex_unlock(&arg->mutex);
	pthread_exit((void*)NULL);
}
