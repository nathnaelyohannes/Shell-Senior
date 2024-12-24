CC = gcc
CFLAGS = -Wall -Wextra -g

# Targets
all: d8sh

d8sh: d8sh.o parser.tab.o lexer.o executor.o
	$(CC) $(CFLAGS) -o d8sh d8sh.o parser.tab.o lexer.o executor.o -lreadline

executor.o: executor.c executor.h command.h
	$(CC) $(CFLAGS) -c executor.c

d8sh.o: d8sh.c parser.tab.h lexer.h
	$(CC) $(CFLAGS) -c d8sh.c

parser.tab.o: parser.tab.c
	$(CC) $(CFLAGS) -c parser.tab.c

lexer.o: lexer.c lexer.h
	$(CC) $(CFLAGS) -c lexer.c

clean:
	rm -f *.o d8sh
