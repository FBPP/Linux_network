all: server client

server: server.o wrap.o
	gcc server.o wrap.o -o server -Wall -g
client: client.o wrap.o
	gcc client.o wrap.o -o client -Wall -g
%.o:%.c
	gcc -c $< -Wall
.PHONY: clean all
clean:
	-rm -rf server client *.o

