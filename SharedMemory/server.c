#include "header.h"

int main() {
	//공유메모리를 위한 구조체
	struct sh_use_st *shmsg[NUM_OF_SHM];

	//시간측정을 위한 구조체
	struct timespec start, stop;
	double accum;

	//시간측정 시작 전 클라이언트 실행을 위한 일시정지
	printf("Press <Enter> to start!\n");
	getchar();

	//시간측정 시작
	if(clock_gettime(CLOCK_MONOTONIC, &start) == -1)
		printf("clock gettime failed\n");

	//공유메모리 변수  생성
	for (int i = 0; i < NUM_OF_SHM; i++)
		shmsg[i] = make_shm(KEY_NUM - i);

	//클라이언트가 전송하는 파일 서버 디렉토리에 백업 및 재전송
	strcpy(shmsg[0]->recvFileName, "directory_server/backup_from_1");
	create_thread(recv_thread, shmsg[0]);
	strcpy(shmsg[1]->sendFileName, "directory_server/backup_from_1");
	create_thread(send_thread, shmsg[1]);

	strcpy(shmsg[1]->recvFileName, "directory_server/backup_from_2");
	create_thread(recv_thread, shmsg[1]);
	strcpy(shmsg[0]->sendFileName, "directory_server/backup_from_2");
	create_thread(send_thread, shmsg[0]);

	//두 클라이언트 모두 종료될 때까지 대기 후 공유메모리 제거 및 시간측정 종료
	while(1){
		if (shmsg[0]->end_flag == 2 && shmsg[1]->end_flag == 2){
			for(int i =0; i < NUM_OF_SHM; i++)
				destroy_shm(shmsg[i]);
			if (clock_gettime(CLOCK_MONOTONIC, &stop) == -1)
				printf("clock gettime failed\n");
			break;
		}
	}

	//시간 계산 및 출력
	accum = (stop.tv_sec - start.tv_sec) + (double)(stop.tv_nsec - start.tv_nsec) / (double)1000000000L;
	printf("프로그램 수행 시간 : %.9fs \n", accum);

    	return 0;
}
