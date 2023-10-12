all: server.o client.o thread.o utils.o
	gcc -o server server.o thread.o utils.o
	gcc -o client client.o thread.o utils.o

utils.o: utils.h utils.c
	gcc -c utils.c

thread.o: thread.h thread.c utils.o
	gcc -c thread.c

server.o: server.c thread.o
	gcc -c server.c

client.o: client.c thread.o
	gcc -c client.c

server: server.o thread.o utils.o
	gcc -o server server.o thread.o utils.o

client: client.o thread.o utils.o
	gcc -o client client.o thread.o utils.o

clean:
	rm -f *.o