.PHONY:clean all
CC=gcc
CFLAGS=-Wall
BIN=cli srv
all:$(BIN)
%.o:%.c
	$(CC) $(CFLAGS) -c $< -o $@
cli:cli.o sysutil.o
	$(CC) $(CFLAGS) $^ -o $@
clean:
	rm -rf *.o $(BIN)
