.PHONY:clean all
CC=gcc
CFLAGS=-Wall
BIN=echocli echosrv test
all:$(BIN)
%.o:%.c
	$(CC) $(CFLAGS) -c $< -o $@
clean:
	rm -rf *.o $(BIN)
