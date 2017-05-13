
all: main

main: LineParser.o
	gcc -g  -Wall LineParser.o -o main

LineParser.o: LineParser.c  LineParser.h
	gcc -g  -Wall -c LineParser.c -o LineParser.o

.PHONY:
	clean

clean:
	rm -f ./*.o  main