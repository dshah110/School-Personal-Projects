CC=gcc
CFLAGS=-Wall -Iincludes -Wextra -std=gnu99 -ggdb #-Wno-sign-compare
LDLIBS=-lcrypto -lm -lcurl

all: client

client: client.c 

clean:
	rm -rf *~ *.o client client.dSYM odyssey.txt 2022-04-04-raspios-bullseye-armhf.img.xz

.PHONY : clean all
