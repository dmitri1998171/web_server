BIN_S = server
SRC_S = src/server.cpp
BIN_WA = web_app
SRC_WA = src/web_app.cpp

LDFLAGS :=
CFLAGS := -pthread

CC := g++

$(BIN_S): $(SRC_S)
	clear && $(CC) $(SRC_S) $(CFLAGS) $(LDFLAGS) -o bin/$(BIN_S)

$(BIN_WA): $(SRC_WA)
	clear && $(CC) $(SRC_WA) $(CFLAGS) $(LDFLAGS) -o bin/$(BIN_WA)

clean:
	rm -rf bin/* 