#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <argp.h>
#include <unistd.h>
#include <time.h>
#include <stdbool.h>
#include <math.h>
#include <poll.h>
#include <signal.h>
#include <openssl/evp.h>
#include <openssl/sha.h>
#include <curl/curl.h>
#include <assert.h>
#include <openssl/evp.h>
#include <pthread.h>

/*
#include "torrent.c"
#include "utilities/utilities.h"
#include "heapless-bencode/bencode.c"
#include "heapless-bencode/bencode.h"
#include "client.h" */


typedef struct peer_thread{
	char *pstr_len;
    char *pstr;
    char *reserved;
    char *info_hash;

	struct peer *this_peer;
	struct torrent *torrent_info;
	unsigned char *i_have;
	unsigned char *they_have;
	
	pthread_t thread_id;
}peer_thread;

//sends the socket a keep alive message
//keep-alive: <len=0000>
int send_keep_alive(int socket);

//sends choking msg
//choke: <len=0001><id=0>
int send_choke(int socket);

//sends not choking msg
//unchoke: <len=0001><id=1>
int send_not_choke(int socket);

//sends interested msg
//interested: <len=0001><id=2>
int send_interested(int socket);

//sends not interested msg
//not interested: <len=0001><id=3>
int send_not_interested(int socket);

//sends have piece msg
//have: <len=0005><id=4><piece index>
int send_have_piece(int socket, int index);

//sends bitfield of all pieces, each bit is a piece we have or don't have
//bitfield: <len=0001+X><id=5><bitfield>
int send_bitfield(int socket, int length, uint8_t *payload);

//sends request msg for a particular piece 
//request: <len=0013><id=6><index><begin><length>
int send_request(int socket, int index, int begin, int length);

//sends a piece to the peer
//piece: <len=0009+X><id=7><index><begin><block>
int send_piece(int socket, int index, int begin, uint8_t *block);

//sends cancel message for a requested block
//cancel: <len=0013><id=8><index><begin><length>
int send_cancel(int socket, int index, int begin, int length);

void handle(int socket, struct peer *pr);


void handle_choke(struct peer *pr);
void handle_unchoke(struct peer *pr);
void handle_interested(struct peer *pr);
void handle_not_interested(struct peer *pr);
void handle_have(int socket, struct peer *pr);
void handle_bitfield(int socket, int length, struct peer *pr);
void handle_request(int socket);
void handle_piece(struct peer *pr, int length);
void write_to_file(FILE *fp, int index, int offset, int length, uint8_t *payload);