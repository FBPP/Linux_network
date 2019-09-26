all: server client

server: server.o wrap.o
	gcc server.o wrap.o -pthread -o server -Wall
client: client.o wrap.o
	gcc client.o wrap.o -pthread -o client -Wall
%.o:%.c
	gcc -c $< -Wall
.PHONY: clean all
clean:
	-rm -rf server client *.o

