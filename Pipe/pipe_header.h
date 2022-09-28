#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>
#include <stdbool.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/mman.h>
#include <pthread.h>
#include <time.h>

#define BUFSIZE 2048
#define NUM_THREADS 3
#define BILLION 1000000000L
#define READ 0
#define WRITE 1
#define SERVER_START 2
#define CLIENT1_START 3
#define CLIENT2_START 4

void client1(int p1[2], int p2[2]);                  // 클라이언트 1번 실행 함수
void client2(int p1[2], int p2[2]);                  // 클라이언트 2번 실행 함수
void server(int p1[2][2], int p2[2][2]);             // 서버 실행 함수
void sendFile(int p[2]);                             // 파일을 버퍼 단위로 파이프와 연결된 상대방에게 송신
void recvFile(int p[2]);                             // 파일을 버퍼 단위로 파이프로 연결된 상대방으로부터 수신
void readFile();                                     // 파일을 읽기 위해 파일 포인터 set
int getFileLen();                                    // 파일의 전체 크기를 알아내는 함수
void waitForSet();                                   //

void* sendThread(void*);                             // 쓰레드 함수: 멀티미디어 파일을 파이프를 통해 send
void* recvThread(void*);                             // 쓰레드 함수: 멀티미디어 파일을 파이프를 통해 recv
void* waitThread(void*);                             // 쓰레드 함수: 파일의 상호 교환을 위한 준비가 완료될 때까지 wait
void createThread(void*, void*);                     // 쓰레드 생성 함수
