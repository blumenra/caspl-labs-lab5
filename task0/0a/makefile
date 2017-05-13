
all: main

main: LineParser.o
	gcc -g -m32 -Wall LineParser.o -o main

LineParser.o: LineParser.c  LineParser.h
	gcc -g -m32 -Wall -c LineParser.c -o LineParser.o

.PHONY:
	clean

clean:
	rm -f ./*.o  main