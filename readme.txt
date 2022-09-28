UNIX시스템프로그래밍 설계과제물

프로젝트명: 멀티미디어 실시간 전송 시스템
팀명: 몽쉘
팀원: 2017154040 한예성
      2017154010 김정현
      2017164031 이승준
제출일: 2021년 12월 9일

[프로젝트 설명]
여러 개의 클라이언트가 서버에 접속하여 대용량의 멀티미디어 데이터(사진)을 상호 전송할 수 있는 프로그램으로
같은 프로그램을 3개의 IPC(shared memory, message passing, pipe)를 이용하여 구현하여
IPC 기법에 따른 성능의 차이를 살펴보고자 함.
프로세스 내에는 3개의 쓰레드가 존재하여 Synchronization tool을 통해 경쟁상태가 일어나지 않도록 함.

[소스코드]
1. Shared Memory
        소스파일: server.c client1.c client2.c header.h header.c
        실행파일: server.out client1.out client2.out
2. Message Passing
        소스파일: message_passing.c , unix_header.h
        실행파일: ./message_passing
3. Pipe
        소스파일: pipe.c pipe_header.h
        실행파일: ./pipe

[컴파일]
make 명령을 통해 컴파일

[실행방법]
1. Shared Memory
 (1) make 명령어로 컴파일
 (2) ./server.out 명령어로 서버 실행 후 ./client1.out, ./client2.out 명령어로 클라이언트 실행.
 (3) 클라이언트 두개 모두 실행되면 서버에서 엔터키 입력 
 make clean 입력 시 make 명령어로 생성된 오브젝트 파일, 실행 파일 삭제
 make clean_datas 입력 시 프로그램 실행하여 생성된 백업파일, 전송파일 삭제

2. Message Passing
 ./message_passing 명령으로 실행
 make clean 수행 시 make 컴파일로 만들어진 파일 삭제

3. Pipe
        ./pipe 명령으로 실행
   make clean 명령 수행 시 make 컴파일로 만들어진 파일 삭제