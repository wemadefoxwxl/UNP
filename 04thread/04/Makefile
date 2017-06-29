.PHONY:clean all
CC=gcc
CFLAGS=-Wall -pthread -std=c99
BIN=pctest
all:$(BIN)
%.o:%.c
	$(CC) $(CFLAGS) -c $< -o $@
clean:
	rm -rf *.o $(BIN)