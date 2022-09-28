#include "header.h"

int main() {
	//공유메모리 변수 생성
	struct sh_use_st *shmsg = make_shm(KEY_NUM);
	init_struct(shmsg);

	strcpy(shmsg->sendFileName, "directory_client1/dataA.jpg");
	create_thread(send_file, shmsg);
	strcpy(shmsg->recvFileName, "directory_client1/recv_from_client2.jpg");
	create_thread(recv_file, shmsg);

	//공유 메모리 변수를 이용해 작업 종료 알림
	shmsg->end_flag = 2;

    	return 0;
}
