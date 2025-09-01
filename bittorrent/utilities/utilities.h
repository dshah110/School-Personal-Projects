#include <stdint.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <pthread.h>

extern int debug;

typedef enum {
  DOWNLOADED = 1,
  IN_PROGRESS = 2,
  EMPTY = 0
}status;

/*Taken from libcurl documentation*/
struct MemoryStruct {
  char *memory;
  size_t size;
};

typedef struct piece {
    int status;
    int num_blocks; //file->info->piece_length/BLOCK_SIZE
    int blocks_recieved; //maybe itll help keep track
    uint8_t *blocks; //=malloc(num_offsets)
    int bytes_last_block;
    //mutex_lock_t lock;
} piece;

typedef struct peer_state {
  int am_interested;
  int am_choking;
  int peer_interested;
  int peer_choking;
} peer_state;

typedef struct peer {
    char *addr;
    int port;

    void *id;
    int socket;
    int current_piece;
    struct piece *pieces; //malloc num of pieces
    struct peer_state states;
} peer;


typedef struct tracker_response {
    long int interval;
    char *tracker_id;
    long int complete;
    long int incomplete;

    struct peer *peers; //multiple peers
    int num_peers;
    int compact;

} tracker_response;

int readBytes(int socket, size_t length, void* buffer);

int sendBytes(int socket, size_t length, void* buffer);

static size_t WriteMemoryCallback(void *contents, size_t size, size_t nmemb, void *userp);

void parse_peer_list(bencode_t *peers, struct tracker_response *resp);

struct tracker_response *parse_tracker_response(int compact, char *response, int length);

struct tracker_response *get_response(int compact, int port, struct torrent *file);

void print_response(struct tracker_response *resp);

float diff(struct timeval t0, struct timeval t1);