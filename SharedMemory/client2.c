#include "header.h"

int main() {
	//공유메모리 변수 생성
	struct sh_use_st *shmsg = make_shm(KEY_NUM - 1);
	init_struct(shmsg);

	strcpy(shmsg->recvFileName, "directory_client2/recv_from_client1.jpg");
	create_thread(recv_file, shmsg);
	strcpy(shmsg->sendFileName, "directory_client2/dataB.jpg");
	create_thread(send_file, shmsg);

	//공유메모리 변수로 작업 종료 알림
	shmsg->end_flag = 2;

    	return 0;
}
