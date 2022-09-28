// UNIX시스템프로그래밍 6팀 - pipe

#include "pipe_header.h"

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;   // Synchronization tool: mutex
pthread_mutex_t f_mutex = PTHREAD_MUTEX_INITIALIZER; 

static int* flag;

typedef struct info {
    FILE* fp;                // 파일을 가리키는 포인터
    char* filename;          // 접근하고자 하는 파일의 이름
    int* p;                  // 사용하고자 하는 파이프
} INFO;

INFO info;                   // 클라이언트 및 서버가 사용해야 할 정보를 담은 구조체
char buf[BUFSIZE];           // 파일의 일부를 일시적으로 저장하는 버퍼

int main() {
    int p1[2][2];            // 클라이언트1 ↔ 서버: p1[0], p2[0]을 이용하여 통신 수행           
    int p2[2][2];            // 서버 ↔ 클라이언트2: p1[1], p2[1]을 이용하여 통신 수행
    int pid[2];              // 프로세스 ID

    // 파이프 구성
    // |          | p1[0]  |          |  p1[1] |          |
    // |          | -----> |          | -----> |          |
    // |  클라 1  |        |   서버   |        |  클라 2  |
    // |          | <----- |          | <----- |          |
    // |          | p2[0]  |          |  p2[1] |          |

    // 시간 측정을 위한 변수
    struct timespec start, stop;
    double accum;

    // 시간 측정 시작
    if (clock_gettime(CLOCK_MONOTONIC, &start) == -1) {
        fprintf(stderr, "ERROR! clock gettime");
        return EXIT_FAILURE;
    }

    // 파이프를 이용한 멀티미디어 데이터 교환 실행
    // 파이프 생성 오류 시 대처
    if (pipe(p1[0]) == -1 || pipe(p1[1]) == -1) {
        fprintf(stderr, "ERROR! Pipe Failed");
        return 1;
    }
    if (pipe(p2[0]) == -1 || pipe(p2[1]) == -1) {
        fprintf(stderr, "ERROR! Pipe Failed");
        return 1;
    }

    // 클라이언트와 서버 간 전역 변수 공유
    flag = mmap(NULL, sizeof * flag, PROT_READ | \
        PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    *flag = 0;

    // 클라이언트 1번 실행
    pid[0] = fork();
    switch (pid[0]) {
    case -1:
        fprintf(stderr, "ERROR! fork call");
        return 1;
    case 0:
        client1(p1[0], p2[0]);     // 클라이언트1 - p1[0]: WRITE용, p2[0]: READ용
        return 0;
    }

    // 클라이언트 2번 실행
    pid[1] = fork();
    switch (pid[1]) {
    case -1:
        fprintf(stderr, "ERROR! fork call");
        return 1;
    case 0:
        client2(p1[1], p2[1]);     // 클라이언트2 - p1[1]: READ용, p2[1]: WRITE용
        return 0;
    }

    // 서버 실행
    server(p1, p2);

    // 클라이언트의 동작이 모두 완료될 때까지 대기
    wait(&pid[0]);
    wait(&pid[1]);

    // 공유변수에 할당된 메모리 해제
    munmap(NULL, sizeof * flag);

    // 파일 교환이 끝났으므로 시간 측정 종료
    if (clock_gettime(CLOCK_MONOTONIC, &stop) == -1) {
        fprintf(stderr, "ERROR! clock gettime");
        return EXIT_FAILURE;
    }

    // 측정한 시간 출력
    accum = (stop.tv_sec - start.tv_sec) + (double)(stop.tv_nsec - start.tv_nsec) / (double)BILLION;
    printf("프로그램 수행 시간: %.9fs\n", accum);
    return 0;
}


// 쓰레드 함수: send 전용
void* sendThread(void* arg) {
    int* pip = (int*)arg;

    pthread_mutex_lock(&mutex);

    // 필요 없는 파이프 close
    close(pip[READ]);
    // 멀티미디어 데이터
    readFile();

    // 파일의 크기 확인
    // printf("size: %d\n", getFileLen(fp));

    // 파일을 파이프에 연결된 상대방에게 전송
    sendFile(pip);
    // 다 쓴 파이프 및 파일 포인터 close
    close(pip[WRITE]);
    fclose(info.fp);

    pthread_mutex_unlock(&mutex);
    pthread_exit((void*)NULL);
}

// 쓰레드 함수: recv 전용
void* recvThread(void* arg) {
    int* pip = (int*)arg;

    pthread_mutex_lock(&mutex);

    // 필요 없는 파이프 close
    close(pip[WRITE]);
    // Output 파일
    info.fp = fopen(info.filename, "wb+");
    // 서버로부터 바든 파일을 새롭게 작성
    recvFile(pip);
    // 다 쓴 파이프 및 파일 포인터 close
    close(pip[READ]);
    fclose(info.fp);

    pthread_mutex_unlock(&mutex);
    pthread_exit((void*)NULL);
}

// 쓰레드 함수: wait 전용
void* waitThread(void* arg) {
    int target = *((int*)arg);
    
    pthread_mutex_lock(&mutex);
    
    // target이 될 때까지 wait
    while ((*flag) != target) {
        // sleep(1);
    }
    
    pthread_mutex_unlock(&mutex);
    pthread_exit((void*)NULL);
}

// 플래그 값 설정
void setFlag(int n) {
    pthread_mutex_lock(&f_mutex);
    (*flag) += n;
    pthread_mutex_unlock(&f_mutex);
}

// 쓰레드 생성 함수
void createThread(void* func, void* arg) {
    pthread_t thread;
    int thr;
    if ((thr = pthread_create(&thread, NULL, func, (void*)arg)) < 0) {
        fprintf(stderr, "ERROR! pthread create error");
	exit(1);
    }
    pthread_join(thread, NULL);
}

// 클라이언트 1번 실행 함수
void client1(int p1[2], int p2[2]) {
    // 클라이언트가 다시 작업을 수행하는 것을 제어
    int target = CLIENT1_START;

    printf("클라이언트 1번: 파일 send 시작\n");

    // 멀티미디어 데이터 A를 서버에 등록
    info.filename = "./clientA/dataA.jpg";
    createThread(sendThread, (void*)p1);
    printf("클라이언트 1번: 파일 send 완료\n");

    // 받을 데이터가 서버에 온전히 저장될 때까지 대기
    createThread(waitThread, (void*)&target);

    // 서버에서 멀티미디어 데이터 B를 받아서 저장
    info.filename = "./clientA/dataB.jpg";
    createThread(recvThread, (void*)p2);
    printf("클라이언트 1번: 파일 recv 완료\n");

    return;
}

// 클라이언트 2번 실행 함수
void client2(int p1[2], int p2[2]) {
    // 클라이언트가 다시 작업을 수행하는 것을 제어
    int target = CLIENT2_START;

    printf("클라이언트 2번: 파일 send 시작\n");

    // 멀티미디어 데이터 A를 서버에 등록
    info.filename = "./clientB/dataB.jpg";
    createThread(sendThread, (void*)p2);
    printf("클라이언트 2번: 파일 send 완료\n");

    // 받을 데이터가 서버에 온전히 저장될 때까지 대기
    createThread(waitThread, (void*)&target);

    // 서버에서 멀티미디어 데이터 B를 받아서 저장
    info.filename = "./clientB/dataA.jpg";
    createThread(recvThread, (void*)p1);
    printf("클라이언트 2번: 파일 recv 완료\n");

    return;
}

// server 실행 함수
void server(int p1[2][2], int p2[2][2]) {
    // 서버가 다시 작업을 수행하는 것을 제어
    int target = SERVER_START;
    
    printf("서버: 서버 recv 대기 중...\n");

    // 클라이언트 측에서 데이터를 보낼 때까지 대기
    createThread(waitThread, (void*)&target);

    // 클라이언트 A -> 서버(쓰레드1)
    info.filename = "./server/serverA.jpg";
    createThread(recvThread, (void*)p1[0]);
    printf("서버: 파일 A recv 완료\n");

    // 클라이언트 B -> 서버(쓰레드2)
    info.filename = "./server/serverB.jpg";
    createThread(recvThread, (void*)p2[1]);
    printf("서버: 파일 B recv 완료\n");
    
    // 서버 -> 클라이언트 A(쓰레드3)
    info.filename = "./server/serverB.jpg";
    createThread(sendThread, (void*)p2[0]);
    printf("서버: 파일 A send 완료\n");

    // 서버 -> 클라이언트 B(쓰레드4)
    info.filename = "./server/serverA.jpg";
    createThread(sendThread, (void*)p1[1]);
    printf("서버: 파일 B send 완료\n");

    return;
}

// 파일을 버퍼 단위로 상대방에게 송신
void sendFile(int p[2]) {
    int cnt = 0;
    int total = 0;
    int tmp = 0;

    // 사용할 버퍼 초기화
    memset(buf, 0, sizeof(buf));

    // 파일을 버퍼 사이즈씩 읽어서 파이프를 통해 상대방에게 send
    while (feof(info.fp) == 0) {
        cnt = fread(buf, 1, BUFSIZE, info.fp);
        write(p[WRITE], buf, cnt);
        total += cnt;
	if (tmp == 0) {
            tmp = 1;
	    setFlag(1);
	// printf("%d\n", *flag);
	}
    }

    printf("total: %d..........", total);
    return;
}

// 파일을 버퍼 단위로 상대방으로부터 수신
void recvFile(int p[2]) {
    int cnt = 0;
    int total = 0;

    // 파이프의 O_NONBLOCK 플래그 설정
    if (fcntl(p[READ], F_SETFL, O_NONBLOCK) == -1) {
        fprintf(stderr, "ERROR! fcntl call");
        return;
    }

    // 사용할 버퍼 초기화
    memset(buf, 0, sizeof(buf));

    while (true) {
        // 파이프로부터 데이터를 읽어서 버퍼에 저장
        if (cnt < 0) {
            break;
        }
        cnt = read(p[READ], buf, BUFSIZE);
        if (cnt == -1) {
            // 파이프에 아무것도 없는지 검사
            if (errno == EAGAIN) {
                continue;
                // printf("해당 파이프에 아무것도 없습니다...\n");
            }
            // 그렇지 않다면 pipe로부터 데이터를 읽는 과정에서 오류가 발생한 것
            else {
                fprintf(stderr, "ERROR! pipe read call");
                exit(1);
            }
        }
        // 읽은 데이터의 양이 0이라면 파이프가 닫힌 것을 의미
        else if (cnt == 0) {
            // printf("파이프가 닫혀 있어 더 이상 읽을 내용이 없습니다...\n");
            break;
        }

        // 버퍼의 내용을 파일로 저장
        fwrite(buf, cnt, 1, info.fp);
        total += cnt;
    }

    printf("total: %d..........", total);
    return;
}

// 파일을 읽기 위해 파일 포인터 set
void readFile() {
    if (!(info.fp = fopen(info.filename, "rb"))) {
        perror("ERROR! file open error");
        exit(EXIT_FAILURE);
    }
}

// 파일의 전체 크기를 알아내는 함수
int getFileLen() {
    int len;

    fseek(info.fp, 0, SEEK_END);
    len = ftell(info.fp);
    fseek(info.fp, 0, SEEK_SET);

    return len;
}


