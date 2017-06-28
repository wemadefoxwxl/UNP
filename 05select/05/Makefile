.PHONY:clean all
CC=gcc
CFLAGS=-Wall
BIN=nofile_limit echosrv conntest
all:$(BIN)
%.0:%.c
	$(CC) $(CFLAGS) -c $< -o $@
clean:
	rm -rf *.o $(BIN)
