.PHONY:clean all
CC=gcc
CFLAGS=-Wall 
BIN=01byteorder 02addr
all:$(BIN)
%.o:%.c
	$(CC) $(CFLAGS) -c $< -o $@
clean:
	rm -rf *.o $(BIN)