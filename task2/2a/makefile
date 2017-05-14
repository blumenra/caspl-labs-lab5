
all: main

main: JobControl.o
	gcc -g  -Wall JobControl.o -o main

JobControl.o: JobControl.c  JobControl.h
	gcc -g  -Wall -c JobControl.c -o JobControl.o

.PHONY:
	clean

clean:
	rm -f ./*.o  main