.PHONY:clean all
CC=gcc
CFLAGS=-Wall
BIN=echosrv echocli echosrv2 
all:$(BIN)
%.o:%.c
	$(CC) $(CFLAGS) -c $< -o $@
clean:
	rm -f *.o $(BIN)