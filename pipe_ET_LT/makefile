all:LT ET 

LT: LT.o wrap.o
	gcc LT.o wrap.o -o LT -Wall -g
ET: ET.o wrap.o
	gcc ET.o wrap.o -o ET -Wall -g
%.o:%.c
	gcc -c $< -Wall
.PHONY: clean all
clean:
	-rm -rf LT ET *.o

