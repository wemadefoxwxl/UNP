.PHONY:clean all
CC=gcc
CFLAGS=-Wall -pthread
BIN=threadattr tsd
all:$(BIN)
%.o:%.c
	$(CC) $(CFLAGS) -c $< -o $@
clean:
	rm -rf *.o $(BIN)