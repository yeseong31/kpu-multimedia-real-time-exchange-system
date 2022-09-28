CC=gcc
CFLAGS=-g -Wall
OBJS=server.o header.o client1.o client2.o
TARGET=app.out

all: server.out client1.out client2.out

server.out: server.o header.o
	$(CC) -o $@ server.o header.o -lpthread
client1.out: client1.o header.o
	$(CC) -o $@ client1.o header.o -lpthread
client2.out: client2.o header.o
	$(CC) -o $@ client2.o header.o -lpthread

server.o: header.h server.c
client1.o: header.h client1.c
client2.o: header.h client2.c
header.o: header.h header.c

clean:
	rm -f *.o
	rm -f *.out

clean_backups:
	rm directory_server/backup_from_1
	rm directory_server/backup_from_2

clean_datas:
	rm directory_client1/recv_from_client2.jpg
	rm directory_client2/recv_from_client1.jpg
	rm directory_server/backup_from_1
	rm directory_server/backup_from_2
