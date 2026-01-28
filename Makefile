CC=gcc
CFLAGS=-Wall -Wextra -Werror -std=c2x -I./include -D_GNU_SOURCE
DEBFLAGS=-g
LINKERFLAGS=-lserver -lhashtable -llogger -Wl,-rpath,'$$ORIGIN/../so'

SRCDIR=./src
BINDIR=./bin
SODIR=./so

SRC=./main.c
SERVER_SRCS=$(SRCDIR)/server.c
HASHTABLE_SRCS=$(SRCDIR)/server_hashtable.c
LOGGER_SRCS=$(SRCDIR)/logger.c

SERVER_SO=$(SODIR)/libserver.so
HASHTABLE_SO=$(SODIR)/libhashtable.so
LOGGER_SO=$(SODIR)/liblogger.so

.PHONY: all build-libs debug clean

all: build-libs
	@mkdir -p $(BINDIR)
	$(CC) $(CFLAGS) $(SRC) -o $(BINDIR)/server -L$(SODIR) $(LINKERFLAGS)

build-libs:
	@mkdir -p $(SODIR)
	$(CC) -shared -o $(SERVER_SO) -fPIC $(SERVER_SRCS) $(CFLAGS)
	$(CC) -shared -o $(HASHTABLE_SO) -fPIC $(HASHTABLE_SRCS) $(CFLAGS)
	$(CC) -shared -o $(LOGGER_SO) -fPIC $(LOGGER_SRCS) $(CFLAGS)

debug: build-libs
	@mkdir -p $(BINDIR)
	$(CC) $(CFLAGS) $(DEBFLAGS) $(SRC) -o $(BINDIR)/debug_server -L$(SODIR) $(LINKERFLAGS)

clean:
	rm -rf $(BINDIR)/*
	rm -rf $(SODIR)/*