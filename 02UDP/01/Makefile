.PHONY:clean all
CC=gcc
BIN=echocli echosrv trunc
all:$(BIN)
%.o:%.c
	$(CC)  -c $< -o $@
clean:
	rm -rf *.o $(BIN)
