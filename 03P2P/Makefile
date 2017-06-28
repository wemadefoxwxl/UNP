.PHONY:clean all
CC=gcc
CFLAGS=-Wall
BIN=p2pcli p2psrv
all:$(BIN)
%.o:%.c
	$(CC) $(CFLAGS) -c $< -o $@
clean:
	rm -rf *.o $(BIN)