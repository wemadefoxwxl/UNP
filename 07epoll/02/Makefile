.PHONY:clean all
CC=g++
BIN=echocli echosrv
all:$(BIN)
%.o:%.cpp
	$(CC) -c $< -o $@
clean:
	rm -rf $(BIN) *.o