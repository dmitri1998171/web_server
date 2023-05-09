BIN_S = server
SRC_S = src/server.cpp
LDFLAGS :=
CFLAGS := -pthread

CC := g++

$(BIN_S): $(SRC_S)
	clear && $(CC) $(SRC_S) $(CFLAGS) $(LDFLAGS) -o bin/$(BIN_S)

clean:
	rm -rf bin/* 