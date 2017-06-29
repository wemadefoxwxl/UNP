.PHONY:clean all
CC=gcc
CFLAGS=-Wall -lpthread
BIN=thread echocli echosrv
all:$(BIN)
%.0:%.c
	$(CC) $(CFLAGS) -c $< -o $@
clean:
	rm -rf *.o $(BIN)
