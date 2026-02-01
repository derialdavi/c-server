CC=gcc
CFLAGS=-Wall -Wextra -Werror -std=c2x -I./include -D_GNU_SOURCE -fPIC
DEBFLAGS=-g
LINKERFLAGS=-lserver -lhashtable -llogger -Wl,-rpath,'$$ORIGIN/../so'

SRCDIR=./src
BINDIR=./bin
SODIR=./so

SRC=./main.c
SERVER_SRCS=$(SRCDIR)/server.c
REQUEST_SRCS=$(SRCDIR)/request.c
HASHTABLE_SRCS=$(SRCDIR)/server_hashtable.c
LOGGER_SRCS=$(SRCDIR)/logger.c

SERVER_SO=$(SODIR)/libserver.so
REQUEST_SO=$(SODIR)/librequest.so
HASHTABLE_SO=$(SODIR)/libhashtable.so
LOGGER_SO=$(SODIR)/liblogger.so

.PHONY: all debug clean

all: build-libs
	@mkdir -p $(BINDIR)
	$(CC) $(CFLAGS) $(SRC) -o $(BINDIR)/server -L$(SODIR) $(LINKERFLAGS)

$(SERVER_SO): $(SERVER_SRCS)
	@mkdir -p $(SODIR)
	$(CC) -shared -fPIC $(CFLAGS) $< -o $@

$(REQUEST_SO): $(REQUEST_SRCS)
	@mkdir -p $(SODIR)
	$(CC) -shared -fPIC $(CFLAGS) $< -o $@

$(HASHTABLE_SO): $(HASHTABLE_SRCS)
	@mkdir -p $(SODIR)
	$(CC) -shared -fPIC $(CFLAGS) $< -o $@

$(LOGGER_SO): $(LOGGER_SRCS)
	@mkdir -p $(SODIR)
	$(CC) -shared -fPIC $(CFLAGS) $< -o $@

build-libs: $(SERVER_SO) $(REQUEST_SO) $(HASHTABLE_SO) $(LOGGER_SO)

debug: build-libs
	@mkdir -p $(BINDIR)
	$(CC) $(CFLAGS) $(DEBFLAGS) $(SRC) -o $(BINDIR)/debug_server -L$(SODIR) $(LINKERFLAGS)

clean:
	rm -rf $(BINDIR)/* $(SODIR)/*
