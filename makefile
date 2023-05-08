CC := g++
CFLAGS := -pthread

BIN_S = server

SRC_S = src/server.c

$(BIN_S): $(SRC_S)
	clear && $(CC) $(SRC_S) $(CFLAGS) -o bin/$(BIN_S)