.PHONY:clean all
CC=gcc
CFLAGS=-Wall 
BIN=echocli epollsrv conntest
all:$(BIN)
%.0:%.c
	$(CC) $(CFLAGS) -std=c99 -c $< -o $@
%.o:%.cpp
	g++ $(CFLAGS) -c $< -o $@ 
epollsrv:epollsrv.o
	g++ $(CFLAGS) $^ -o $@
clean:
	rm -rf *.o $(BIN)
