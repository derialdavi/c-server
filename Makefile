CC=gcc
CFLAGS=-Wall -Wextra -Werror -std=c2x -I./include -D_GNU_SOURCE
DEBFLAGS=-g
LINKERFLAGS=-lserver -llogger -Wl,-rpath,'$$ORIGIN/../so'

SRCDIR=./src
BINDIR=./bin
SODIR=./so

SRC=./main.c
SERVER_SRCS=$(SRCDIR)/server.c
LOGGER_SRCS=$(SRCDIR)/logger.c

SERVER_SO=$(SODIR)/libserver.so
LOGGER_SO=$(SODIR)/liblogger.so

.PHONY: all build-libs debug clean

all: build-libs
	@mkdir -p $(BINDIR)
	$(CC) $(CFLAGS) $(SRC) -o $(BINDIR)/server -L$(SODIR) $(LINKERFLAGS)

build-libs:
	@mkdir -p $(SODIR)
	$(CC) -shared -o $(SERVER_SO) -fPIC $(SERVER_SRCS) $(CFLAGS)
	$(CC) -shared -o $(LOGGER_SO) -fPIC $(LOGGER_SRCS) $(CFLAGS)

debug: build-libs
	@mkdir -p $(BINDIR)
	$(CC) $(CFLAGS) $(DEBFLAGS) $(SRC) -o $(BINDIR)/debug_server

clean:
	rm -rf $(BINDIR)/*
	rm -rf $(SODIR)/*