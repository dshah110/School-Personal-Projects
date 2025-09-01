CC=gcc
CFLAGS=-Wall -Iincludes -Wextra -std=gnu99 -ggdb
LDLIBS=-lprotobuf-c -lcrypto

all: rpc hashtest chord

chord: chord.c chord.pb-c.c hash.o    

chord.pb-c.c: chord.proto
	protoc-c --c_out=. $<

rpc: rpc.c rpc.pb-c.c

rpc.pb-c.c: rpc.proto
	protoc-c --c_out=. $<

hashtest: hash.o hashtest.o

clean:
	rm -rf *~ *.o rpc hashtest chord

.PHONY : clean all
