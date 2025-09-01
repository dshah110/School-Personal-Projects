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

//#include "heapless-bencode/bencode.h"
#include "utilities/utilities.c"
int num_pieces = 0; 
int piece_length = 0;
int blocks_recieved = 0;
char *file_name;
FILE *fp;
#define BLOCK_SIZE 16348

int connect_to(struct peer peers);

int setup_sock(int port);

struct peer *get_responsive_peers(struct peer *tracker_peers, int num_peers, int *responsive_peers);

int handshake(struct peer pr, unsigned char *infohash, uint8_t *our_id);

