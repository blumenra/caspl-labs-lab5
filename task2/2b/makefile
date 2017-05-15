
all: main

main: JobControl.o  LineParser.o
	gcc -g  -Wall JobControl.o  LineParser.o  -o main

LineParser.o: LineParser.c  LineParser.h
	gcc -g  -Wall -c LineParser.c -o LineParser.o

JobControl.o: JobControl.c  JobControl.h
	gcc -g  -Wall -c JobControl.c -o JobControl.o

.PHONY:
	clean

clean:
	rm -f ./*.o  main