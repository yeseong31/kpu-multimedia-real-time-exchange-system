#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <errno.h>
#include <time.h>

#define BUFSIZE 2048

void makeProcess();     // 클라이언트 프로세스 생성
void child(int msqid);           // 자식 프로세스 수행 코드
void parent();          // 부모 프로세스 수행 코드
FILE* getFilePointer(char* filename);       // 파일 불러오기
void* thread_readFileBuf_child1();
void* thread_readFileBuf_child2();
void* thread_readFileBuf_parent(void* chinfo);
void parent_recv();
void child_recv(int msqid);
void send(char* who, int msqid);


