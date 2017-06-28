.PHONY:clean all
CC=gcc
CFLAGS=-Wall
BIN=echocli echosrv getiplist
all:$(BIN)
%.o:%.c
	$(CC) $(CFLAGS) -c $< -o $@
clean:
	rm -rf *.o $(BIN)