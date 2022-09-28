all: message_passing

message_passing : message_passing.o
	gcc -o message_passing message_passing.c -lpthread
clean:
	rm -f *.o message_passing
