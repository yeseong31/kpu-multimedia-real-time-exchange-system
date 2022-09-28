
/*
 *      [UNIX시스템프로그래밍(02)] 2021학년도 2학기 설계 과제
 *              6팀 몽쉘 - 멀티미디어 실시간 교환 시스템
 *              컴퓨터공학과 2017154040 한예성
 *              컴퓨터공학과 2017154010 김정현
 *              컴퓨터공학과 2017154031 이승준
 */

#include "unix_header.h"
typedef struct{ // 메세지 구조체
    long mtype;
    char buf[BUFSIZE];
    int flag;
}msgBuf;



typedef struct{
        char* name;
        int msqid;
}childInfo;


pthread_mutex_t mutex_parent = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex_child1 = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex_child2 = PTHREAD_MUTEX_INITIALIZER;
pthread_t pth_parent1;
pthread_t pth_parent2;
pthread_t pth_child1;
pthread_t pth_child2;
msgBuf parent_sendMsgBuf1;
msgBuf parent_sendMsgBuf2;
msgBuf parent_recvMsgBuf1;
msgBuf parent_recvMsgBuf2;
msgBuf child_sendMsgBuf1;
msgBuf child_sendMsgBuf2;
FILE* chfp1; // 파일 포인터
FILE* chfp2;
FILE* pfp1; // 부모 프로세스가 사용할 파일 포인터
FILE* pfp2;
int main () {
	struct timespec begin, end;

	clock_gettime(CLOCK_MONOTONIC, &begin); // 시간 측정 시작

	chfp1 = fopen("./child1/dataA.jpg", "rb");
	chfp2 = fopen("./child2/dataB.jpg", "rb");
	makeProcess();
	printf("program finish!\n");

	clock_gettime(CLOCK_MONOTONIC, &end);

	printf("수행 시간 : %.4f\n", (end.tv_sec - begin.tv_sec) + (end.tv_nsec - begin.tv_nsec) / 1000000000.0);
	return 0;
}

void makeProcess() { // 전체적인 프로그램 흐름 실행
    pid_t pid;
    // 클라이언트 1, 2번 생성
    for (int i = 0; i < 2; i++) {
        if ((pid = fork()) == -1) {
            printf("ERROR! fork() error");
            exit(1);
        } else if (pid == 0) {
            child(60068 + i); // 자식 `프로세스 1 에게 fd1 전달
	    exit(1);
        }
    }

    // 여기까지 왔으면 이 프로세스는 '부모'임
    parent();

}

FILE* getFilePointer(char* filename) { // 입력 받은 파일의 파일 포인터를 read 형식으로 반환
   FILE* fp;	
   if (!(fp = fopen(filename, "rb"))) {
      perror("ERROR! file open error");
      exit(EXIT_FAILURE);
   }

   return fp;
}

void parent(){ // 부모 프로세스가 실행하는 함수
	parent_recv();
	send("parent", 60068);
	send("parent", 60069);
}

void parent_recv(){
	msgBuf recvMsgBuf; // 메시지 버퍼를 받을 변수
	key_t ch1_key_id;
	key_t ch2_key_id;
	int flag1 = 0; // 자식 프로세스 1이 모든 전송을 끝냈는지를 확인하는 플래그
	int flag2 = 0; // 자식 프로세스 2가 모든 전송을 끝냈는지 확인하는 플래그

	FILE* fp1 = fopen("./parent/child1_result.jpg", "wb");
	FILE* fp2 = fopen("./parent/child2_result.jpg", "wb");

	ch1_key_id = msgget((key_t)60068, IPC_CREAT|0666);
	ch2_key_id = msgget((key_t)60069, IPC_CREAT|0666);
	int cnt = 0;
	while(1){
		if(flag1 == 1 && flag2 == 1){ // 자식 프로세스들이 모든 전송을 끝냈다면 수신 종료
			break;
		}

		if (flag1 == 0){
			if((cnt = msgrcv(ch1_key_id, (void*)&parent_recvMsgBuf1, sizeof(parent_recvMsgBuf1) - sizeof(long), 1, 0)) == -1){
				perror("ERROR! msgrcv from child1\n");
				exit(1);
			}
			if(parent_recvMsgBuf1.flag == 1){ // 자식 프로세스가 보낸 자료 중 전송을 끝냈다는 플래그가 1이면 수신 중지
				flag1 = 1;
				continue;
			}
			fwrite(parent_recvMsgBuf1.buf, 1, sizeof(parent_recvMsgBuf1.buf), fp1); // 받은 버퍼를 바로 write
			memset((void*)&parent_recvMsgBuf1, 0, sizeof(parent_recvMsgBuf1)); // 메모리 초기화
		}
		if (flag2 == 0){
			if((cnt = msgrcv(ch2_key_id, (void*)&parent_recvMsgBuf2, sizeof(parent_recvMsgBuf2) - sizeof(long), 2, 0)) == -1){
				perror("ERROR! msgrcv from child2\n");
				exit(1);
			}
			if(parent_recvMsgBuf2.flag == 1){
				flag2 = 1;
				continue;
			}
			fwrite(parent_recvMsgBuf2.buf, 1, sizeof(parent_recvMsgBuf2.buf), fp2);
			memset((void*)&parent_recvMsgBuf2, 0, sizeof(parent_recvMsgBuf2));
		}
	}
	fclose(fp1);
	fclose(fp2);

	pfp1 = getFilePointer("./parent/child1_result.jpg");
	pfp2 = getFilePointer("./parent/child2_result.jpg");
}


void child_recv(int msqid) {
    key_t key_id;
    msgBuf recvMsgBuf;
    FILE* fp;
    int ms; // 메시지 큐 타입을 구별하는 변수
    int cnt; // 메시지의 길이 확인
    if(msqid == 60068){ // 60068은 자식 프로세스1, 60069는 자식프로세스 2가 사용하는 key_id
	    fp = fopen("./child1/result_from_child2.jpg", "wb");
	    ms = 3;
    }
    else{
	    fp = fopen("./child2/result_from_child1.jpg", "wb");
	    ms = 4;
    }

    key_id = msgget((key_t)msqid, IPC_CREAT|0666);
    while(1){
        if((cnt = msgrcv(key_id, (void*)&recvMsgBuf, sizeof(recvMsgBuf) - sizeof(long), ms , 0)) == -1){
            	printf("ERROR! msgrcv\n");
            	exit(1);
        }
	if(recvMsgBuf.flag == 1){
		break;
	}
	fwrite(recvMsgBuf.buf, 1, sizeof(recvMsgBuf.buf), fp);
	memset((void*)&recvMsgBuf, 0, sizeof(recvMsgBuf));
    }
}

void child(int msqid) {
    char* childName;
    if(msqid == 60068){
	    childName = "child1";
    }
    else{
	    childName = "child2";
    }
    send(childName, msqid);
    child_recv(msqid);

}

void send(char* who, int msqid){
    key_t key_id;
    int status;
    key_id = msgget((key_t)msqid, IPC_CREAT|0666);
    if (key_id == -1){
        printf("ERROR! msgget\n");
        exit(1);
    }

    while(1){

	   if(strcmp(who, "child1") == 0){
		child_sendMsgBuf1.mtype = 1;
		pthread_create(&pth_child1, NULL, thread_readFileBuf_child1, NULL);
		pthread_join(pth_child1, NULL);
		if(feof(chfp1) != 0){
			child_sendMsgBuf1.flag = 1;
		}
		else{
			child_sendMsgBuf1.flag = 0;
		}
           	if (msgsnd(key_id, (void*)&child_sendMsgBuf1, sizeof(child_sendMsgBuf1) - sizeof(long), 0) == -1){
		   	perror("ERROR! msgsnd()\n");
		   	return;
		}
		if (child_sendMsgBuf1.flag == 1){
			break;
		}
		memset((void*)&child_sendMsgBuf1, 0, sizeof(child_sendMsgBuf1));
		
	   }
	   else if(strcmp(who, "child2") == 0){
		   child_sendMsgBuf2.mtype = 2;
		   pthread_create(&pth_child2, NULL, thread_readFileBuf_child2, NULL);
		   pthread_join(pth_child2, NULL);
		   if(feof(chfp2) != 0){
			   child_sendMsgBuf2.flag = 1;
		   }
		   else{
			   child_sendMsgBuf2.flag = 0;
		   }
		   if( msgsnd(key_id, (void*)&child_sendMsgBuf2, sizeof(child_sendMsgBuf2) - sizeof(long), 0) == -1){
		   	perror("ERROR! msgsnd()\n");
			exit(1);
		    }
		   if (child_sendMsgBuf2.flag == 1){
			   break;
		   }
		   memset((void*)&child_sendMsgBuf2, 0, sizeof(child_sendMsgBuf2));
	   }
	   else if(strcmp(who, "parent") == 0 && msqid == 60068){
		   parent_sendMsgBuf1.mtype = 3;
		   childInfo chinfo;
		   chinfo.name = who;
		   chinfo.msqid = msqid;
		   pthread_create(&pth_parent1, NULL, thread_readFileBuf_parent, (void*)&chinfo);
		   pthread_join(pth_parent1, NULL);

		   if(feof(pfp1) != 0){
			   parent_sendMsgBuf1.flag = 1;
		   }
		   else{
			   parent_sendMsgBuf1.flag = 0;
		   }
		   if(msgsnd(key_id, (void*)&parent_sendMsgBuf1, sizeof(parent_sendMsgBuf1) - sizeof(long), 0) == -1){
			   perror("ERROR!parent1 msgsnd()\n");
			   exit(1);
		   }
		   if(parent_sendMsgBuf1.flag == 1){
			   break;
		   }
		   memset((void*)&parent_sendMsgBuf1, 0,sizeof(msgBuf));
	   }
	   else if (strcmp(who, "parent") == 0 && msqid == 60069){
		   parent_sendMsgBuf2.mtype = 4;
		   childInfo chinfo;
		   chinfo.name = who;
		   chinfo.msqid = msqid;
		   pthread_create(&pth_parent2, NULL, thread_readFileBuf_parent, (void*)&chinfo);
		   pthread_join(pth_parent2, NULL);
		   
		   if(feof(pfp2) != 0){
			   parent_sendMsgBuf2.flag = 1;
		   }
		   else{
			   parent_sendMsgBuf2.flag = 0;
		   }
		   if(msgsnd(key_id, (void*)&parent_sendMsgBuf2, sizeof(parent_sendMsgBuf2) - sizeof(long), 0) == -1){
			   perror("ERROR! parent2 msgsnd()\n");
			   exit(1);
		   }
		   if(parent_sendMsgBuf2.flag == 1){
			   break;
		   }
		   memset((void*)&parent_sendMsgBuf2, 0, sizeof(msgBuf));
	   }

     }
}

void* thread_readFileBuf_child1(){ // 쓰레드를 활용하여 fp로 파일을 읽는 함수
	pthread_mutex_lock(&mutex_child1);
        fread(child_sendMsgBuf1.buf, 1, BUFSIZE, chfp1);
	pthread_mutex_unlock(&mutex_child1);
}

void* thread_readFileBuf_child2(){
	pthread_mutex_lock(&mutex_child2);
	fread(child_sendMsgBuf2.buf, 1, BUFSIZE, chfp2);
	pthread_mutex_unlock(&mutex_child2);
}

void* thread_readFileBuf_parent(void* chinfo){
	pthread_mutex_lock(&mutex_parent);
	childInfo* mychinfo = (childInfo*)chinfo;
	if(mychinfo->msqid == 60068){
		fread(parent_sendMsgBuf1.buf, 1, BUFSIZE, pfp1);
	}


	else if(mychinfo->msqid == 60069){
		fread(parent_sendMsgBuf2.buf, 1, BUFSIZE, pfp2);
	}
	pthread_mutex_unlock(&mutex_parent);
}
