CC=gcc
CFLAGS=-Wall -Iincludes -Wextra -ggdb -std=gnu99#-std=c99 -ggdb
LDLIBS=-lcrypto
VPATH=src

all: server client

server: server.o 

client: client.o

clean:
	rm -rf *~ *.o server client


.PHONY : clean all
